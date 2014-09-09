#include "block_file.h"
#include "utils.h"

#include <syslog.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>

static const uint64_t s_tag_size = 16;
static const uint64_t s_blocks_per_chunk = s_blocks_per_region * s_regions_per_chunk;
static const uint64_t s_block_header_size = s_tag_size + sizeof(uint32_t);
static const uint64_t s_block_total_size = s_block_header_size + s_bytes_per_block;
static const uint64_t s_region_footer_size = s_tag_size + sizeof(uint32_t) * s_blocks_per_region;
static const uint64_t s_region_total_size = s_blocks_per_region * s_block_total_size + s_region_footer_size;
static const uint64_t s_region_footer_off = s_blocks_per_region * s_block_total_size;
static const uint64_t s_chunk_footer_size = s_tag_size + sizeof(uint32_t) * s_blocks_per_chunk;
static const uint64_t s_chunk_total_size = s_region_total_size * s_regions_per_chunk + s_chunk_footer_size;
static const uint64_t s_chunk_footer_off = s_region_total_size * s_regions_per_chunk;
static const uint64_t s_ivs_per_block = 1;
static const uint64_t s_ivs_per_region = s_blocks_per_region * s_ivs_per_block + 1;
static const uint64_t s_ivs_per_chunk = s_regions_per_chunk * s_ivs_per_region + 1;

struct coordinates 
{
	coordinates(uint64_t _physical) 
		: physical(_physical)
		, chunk_id(physical / s_blocks_per_chunk)
		, bid_chunk(physical - chunk_id * s_blocks_per_chunk)
		, region_id(bid_chunk / s_blocks_per_region)
		, block_id(bid_chunk - region_id * s_blocks_per_region)
		, region_offset(region_id * s_region_total_size)
		, block_offset(region_offset + block_id * s_block_total_size)
		, iv(chunk_id * s_ivs_per_chunk + region_id * s_ivs_per_region + block_id * s_ivs_per_block)
	{}

	uint64_t physical;       // Global linear coordinates
	uint64_t chunk_id;       // Which chunk am I in
	uint64_t bid_chunk;      // Block ID within entire chunk
	uint64_t region_id;      // Which region am I in (within the chunk)
	uint64_t block_id;       // Which block am I in (within the region)
	uint64_t region_offset;  // Start of this region within chunk (in bytes)
	uint64_t block_offset;   // Start of this block within chunk (in bytes)
	uint64_t iv;             // Iv of this block
};

static 
void set_logical(char* buf, uint64_t which, uint32_t logical) 
{
	*((uint32_t*) (buf + s_tag_size + which * sizeof(uint32_t))) = htonl(logical);
}

static 
uint32_t get_logical(char* buf, uint64_t which) 
{
	return ntohl(*((uint32_t*) (buf + which * sizeof(uint32_t))));
}

block_file::block_file(const cipher_key_t& key)
	: m_cipher_ctx(key)
	, m_region_footer(s_region_footer_size)
	, m_chunk_footer(s_chunk_footer_size)
{
}

bool block_file::open(const string& _dir) 
{
	m_dir = _dir;
	// Try for recovery
	DIR *dir = opendir(m_dir.c_str());
	if (dir == NULL && errno == ENOENT) {
		// Nothing there, let's try making it
		if (mkdir(m_dir.c_str(), 0777) == 0) {
			// Worked, try opendir again
			dir = opendir(m_dir.c_str());
		}
	}
	// Open the directory for a scan
	if (dir == NULL) {
		syslog(LOG_ERR, "Unable to open directory: %s", strerror(errno));
		return false;
	}
	// Find lowest and highest entries
	struct dirent *de;
	uint64_t low_chunk = -1;
	uint64_t high_chunk = 0;
	while ((de = readdir(dir)) != NULL) {
		if (de->d_name[0] == '.') {
			continue;
		}
		if (memcmp(de->d_name, "file_", 5) != 0) {
			syslog(LOG_ERR, "Unexpected entry, forget it");
			closedir(dir);
			return false;
		}
		uint64_t num = atoi(de->d_name + 5);
		low_chunk = std::min(low_chunk, num);
		high_chunk = std::max(high_chunk, num);
	}
	closedir(dir);
	// Make sure we didn't err out of the loop
	if (errno != 0) {
		syslog(LOG_ERR, "Error during directory scan: %s", strerror(errno));
		return false;
	}
	// Fix low chunk for empty directory case
	low_chunk = std::min(low_chunk, high_chunk);
	// Verify and open 'complete' files
	for (uint64_t chunk = low_chunk; chunk < high_chunk; chunk++) {
		int fd = ::open(file_name(chunk).c_str(), O_RDONLY);
		if (fd < 0) {
			syslog(LOG_ERR, "Unable to open file: %s, %s", file_name(chunk).c_str(), strerror(errno));
			close();
			return false;
		}
		off_t end = lseek(fd, 0, SEEK_END);
		if (end != s_chunk_total_size) {
			syslog(LOG_ERR, "Non-final file too short, or seek failed: %s", file_name(chunk).c_str());
			::close(fd);
			close();
			return false;
		}
		m_chunks[chunk].fd = fd;
		m_chunks[chunk].size = end;
	}
	// Open / create final file 
	int fd = ::open(file_name(high_chunk).c_str(), O_RDWR | O_CREAT, 0777);
	if (fd < 0) {
		close();
		syslog(LOG_ERR, "Unable to final file: %s", file_name(high_chunk).c_str());
		return false;
	}
	// Get size of final file
	off_t end = lseek(fd, 0, SEEK_END);
	if (end < 0) {
		::close(fd);
		close();
		syslog(LOG_ERR, "Unable to size final file");
		return false;
	}
	uint64_t chunk = high_chunk;
	m_fi = &m_chunks[chunk];
	m_fi->fd = fd;
	if (end == s_chunk_total_size) {
		// Special case for final file also being complete
		m_fi->size = s_chunk_total_size;
		next_chunk(chunk);
		m_next = (chunk + 1) * s_blocks_per_chunk;
		return true;
	}
	
	// Compute record offset based on file size, trucate to a valid size	
	// Do binary search to find valid offset
	// This could be probably be computed some other way, but I'm drunk
	uint64_t low_phy = high_chunk * s_blocks_per_chunk;
	uint64_t high_phy = (high_chunk + 1) * s_blocks_per_chunk;
	while (low_phy + 1 < high_phy) {
		//syslog(LOG_DEBUG, "End = %ju, low_phy = %ju, high_phu = %ju", end, low_phy, high_phy);
		uint64_t mid_phy = (low_phy + high_phy) / 2;
		coordinates c(mid_phy);
		if (c.block_offset > (uint64_t)end) {
			high_phy = mid_phy;
		} else {
			low_phy = mid_phy; 
		}
	}
	// Ok, now truncate
	m_next = low_phy;
	coordinates c(low_phy);
	if ((uint64_t)end > c.block_offset) {
		if (ftruncate(m_fi->fd, m_fi->size) != 0) {
			close();
			syslog(LOG_ERR, "Failed to truncate final file on reload");
			return false;
		}
	}
	m_fi->size = c.block_offset;
	// Ready to go
	return true;
}

void block_file::close()
{
	m_dir = "";
	m_fi = NULL;
	for(auto& kvp : m_chunks) {
		::close(kvp.second.fd);
	}
}

bool block_file::scan(std::function<void (uint64_t, uint32_t)> callback)
{
	//syslog(LOG_DEBUG, "Scanning till %ju", m_next);
	coordinates c(m_next);
	assert(m_chunks.size());
	// Read chunk footers
	for (auto it = m_chunks.begin(); it->first < c.chunk_id; it++) {
		int fd = it->second.fd;
		uint64_t chunk = it->first;
		//syslog(LOG_DEBUG, "Reading footer of chunk %ju", chunk);
		off_t r = lseek(fd, s_chunk_footer_off, SEEK_SET);
		if (r != s_chunk_footer_off) {
			syslog(LOG_ERR, "Couldn't seek to chunk data");
			return false;
		}
		slice_t data(s_chunk_footer_size); 
		if (!read_fully(fd, data.buf(), data.size())) {
			syslog(LOG_ERR, "Couldn't read chunk data");
			return false;
		}
		uint64_t iv = (chunk + 1) * s_ivs_per_chunk - 1;
		if (!simple_dec(iv, data)) {
			syslog(LOG_ERR, "Crypto err in chunk footer scan");
			return false;
		}
		uint64_t base = chunk * s_blocks_per_chunk;
		for (uint64_t boff = 0; boff < s_blocks_per_chunk; boff++) {
			uint32_t logical = get_logical(data.buf() + s_tag_size, boff);
			callback(base + boff, logical);
		} 
	}	
	// Ok, I'm on the last chunk
	int fd = m_chunks[c.chunk_id].fd;
	// Read region footers
	for (uint64_t region = 0; region < c.region_id; region++) {
		//syslog(LOG_DEBUG, "Reading footer of region %ju", region);
		uint64_t roff = region * s_region_total_size + s_region_footer_off;
		off_t r = lseek(fd, roff, SEEK_SET);
		if ((uint64_t)r != roff) {
			syslog(LOG_ERR, "Couldn't seek to region data");
			return false;
		}
		slice_t data(s_region_footer_size); 
		if (!read_fully(fd, data.buf(), data.size())) {
			syslog(LOG_ERR, "Couldn't read region data");
			return false;
		}
		uint64_t rbase = region * s_blocks_per_region;
		uint64_t base = c.chunk_id * s_blocks_per_chunk + rbase;
		uint64_t iv = c.chunk_id * s_ivs_per_chunk + (region + 1) * s_ivs_per_region - 1;
		//syslog(LOG_DEBUG, "Region footer IV = %ju", iv);
		if (!simple_dec(iv, data)) {
			syslog(LOG_ERR, "Crypto err in region footer scan");
			return false;
		}
		for (uint64_t boff = 0; boff < s_blocks_per_region; boff++) {
			uint32_t logical = get_logical(data.buf() + s_tag_size, boff);
			callback(base + boff, logical);
			set_logical(m_chunk_footer.buf(), rbase + boff, logical);
		} 
	}
	// Read left over block headers
	uint64_t base = c.chunk_id * s_blocks_per_chunk + c.region_id * s_blocks_per_region;
	uint64_t off_base = c.region_id * s_region_total_size; 
	uint64_t iv_base = c.chunk_id * s_ivs_per_chunk + c.region_id * s_ivs_per_region;
	for (uint64_t block = 0; block < c.block_id; block++) {
		uint64_t roff = off_base + block * s_block_total_size;
		off_t r = lseek(fd, roff, SEEK_SET);
		if ((uint64_t)r != roff) {
			syslog(LOG_ERR, "Couldn't seek to block meta-data");
			return false;
		}
		slice_t data(s_block_total_size); 
		if (!read_fully(fd, data.buf(), s_block_total_size)) {
			syslog(LOG_ERR, "Couldn't read block meta-data");
			return false;
		}
		if (!simple_dec(iv_base + block, data)) {
			syslog(LOG_ERR, "Crypto err in data scan");
			return false;
		}
		uint32_t logical = get_logical(data.buf() + s_tag_size, 0);
		callback(base + block, logical);
		set_logical(m_chunk_footer.buf(), c.region_id * s_blocks_per_region + block, logical);
		set_logical(m_region_footer.buf(), block, logical);
	}
	return true;
}

bool block_file::remove_old(uint64_t keep_after)
{
	coordinates c(keep_after);
	while (m_chunks.size() && m_chunks.begin()->first < c.chunk_id) {
		//syslog(LOG_DEBUG, "Keep after: %ju, chunk_id = %ju, top = %ju, removing", keep_after, c.chunk_id, m_chunks.begin()->first);
		auto it = m_chunks.begin();
		::close(it->second.fd);
		int r = unlink(file_name(it->first).c_str());
		m_chunks.erase(it);
		if (r != 0) {
			syslog(LOG_ERR, "Unable to rm file");
			return false;
		}
	}
	return true;
}

bool block_file::write_block(uint32_t logical, const rslice_t& block, uint64_t& physical_out)
{
	//syslog(LOG_DEBUG, "Writing logical %u -> physical %ju", logical, m_next);
	assert(block.size() == s_bytes_per_block);
	// Break things down into coordinates
	coordinates c(m_next);

	// Make room for encrypted block
	slice_t block_buf(s_block_total_size);

	// Set logical block and info into current, region, and chunk buffers
	set_logical(block_buf.buf(), 0, logical);
	set_logical(m_region_footer.buf(), c.block_id, logical);
	set_logical(m_chunk_footer.buf(), c.bid_chunk, logical);

	// Do encryption 
	m_cipher_ctx.gcm_set_iv(c.iv);
	m_cipher_ctx.gcm_partial_encrypt(block_buf.slice(s_tag_size, sizeof(uint32_t)));
	m_cipher_ctx.gcm_partial_encrypt(block_buf.slice(s_block_header_size, block.size()), block);
	m_cipher_ctx.gcm_finalize(block_buf.slice(0, s_tag_size));

	// Seek to location for new block
	off_t r = lseek(m_fi->fd, c.block_offset, SEEK_SET);
	if ((uint64_t)r != c.block_offset) {
		syslog(LOG_ERR, "lseek returned invalid result for seek to %ju: %jd, %s", 
			(uintmax_t)c.block_offset, (intmax_t)r, strerror(errno)
		);
		return false;
	}
	// Write block
	if (!write_fully(m_fi->fd, block_buf.buf(), block_buf.size())) {
		syslog(LOG_ERR, "Unable to write block");
		return false;
	}	
	m_fi->size += block_buf.size();
	// If end of region, write region tailer
	if (c.block_id + 1 == s_blocks_per_region) {
		// Do encrypt
		//syslog(LOG_DEBUG, "Writing region footer, iv = %ju", c.iv + 1);
		simple_enc(c.iv + 1, m_region_footer);
		// Do write
		if (!write_fully(m_fi->fd, m_region_footer.buf(), m_region_footer.size())) {
			syslog(LOG_ERR, "Unable to write region footer");
			return false;
		}
		m_fi->size += m_region_footer.size();
	}
	// If end of chunk, write chunk tailer
	if (c.bid_chunk + 1 == s_blocks_per_chunk) {
		// Do encrypt
		//syslog(LOG_DEBUG, "Writing chunk footer, iv = %ju", c.iv + 2);
		simple_enc(c.iv + 2, m_chunk_footer);
		// Do write
		if (!write_fully(m_fi->fd, m_chunk_footer.buf(), m_chunk_footer.size())) {
			syslog(LOG_ERR, "Unable to write chunk footer");
			return false;
		}
		m_fi->size += m_chunk_footer.size();
		// Close old chunk and reopen readonly
		if (!next_chunk(c.chunk_id)) {
			return false;
		}
	}

	// Output physical offset, and move to next block
	physical_out = m_next;
	m_next++;
	return true;
}

bool block_file::read_block(uint64_t physical, rslice_t& block_out, uint32_t& logical_out)
{	
	//syslog(LOG_DEBUG, "Reading physical %ju", physical);
	// Break things down into coordinates
	coordinates c(physical);

	// Find chunk in map
	auto it = m_chunks.find(c.chunk_id);
	if (it == m_chunks.end()) {
		syslog(LOG_ERR, "Trying to read block from invalid chunk");
		return false;
	}
	file_info& fi = it->second;

	// Check that the data is there
	if (c.block_offset + s_block_total_size > fi.size) {
		syslog(LOG_ERR, "Attempt to read block past EOF, offset = %ju, size = %ju", 
			(uintmax_t)c.block_offset, (uintmax_t)fi.size
		);
		return false;
	}
	// Do seek to proper offset
	off_t r = lseek(fi.fd, c.block_offset, SEEK_SET);
	if (r != c.block_offset) {
		syslog(LOG_ERR, "lseek returned invalid result for seek to %ju: %jd", 
			(uintmax_t)c.block_offset, (intmax_t)r
		);
		return false;
	}
	// Do read
	slice_t block_buf(s_block_total_size);
	if (!read_fully(fi.fd, block_buf.buf(), block_buf.size())) {
		syslog(LOG_ERR, "Read of encrypted block failed");
		return false;
	}

	if (!simple_dec(c.iv, block_buf)) {
		return false;
	}
	// Extract address portion
	logical_out = get_logical(block_buf.buf() + s_tag_size, 0);
	//syslog(LOG_DEBUG, "Logical = %u", logical_out);
	// Return data portion
	block_out = block_buf.hrest(s_tag_size + sizeof(uint32_t));
	return true;
}

bool block_file::next_chunk(uint64_t chunk) 
{
	// Close old chunk and reopen readonly
	//syslog(LOG_DEBUG, "Making next chunk");
	::close(m_fi->fd);
	m_fi->fd = ::open(file_name(chunk).c_str(), O_RDONLY);
	if (m_fi->fd < 0) {
		syslog(LOG_ERR, "Unable to reopen final chunk");
		return false;
	}
	// Add new chunk
	int new_fd = ::open(file_name(chunk + 1).c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (new_fd < 0) {
		syslog(LOG_ERR, "Unable to create new file");
		return false;
	}
	m_fi = &m_chunks[chunk + 1];
	m_fi->fd = new_fd;
	m_fi->size = 0;
	
	return true;
}

string block_file::file_name(uint64_t chunk_id)
{
	char filename[50];
	sprintf(filename, "/file_%d", (int) chunk_id); 
	return m_dir + filename;
}

void block_file::simple_enc(uint64_t iv, const slice_t& buf)
{
	//syslog(LOG_DEBUG, "Doing simple_enc, iv = %ju", iv);
	//hexdump(stderr, buf.buf(), buf.size()); 
	// Set IV
	m_cipher_ctx.gcm_set_iv(iv);
	// Do bulk encrypt
	m_cipher_ctx.gcm_partial_encrypt(buf.hrest(s_tag_size));
	// Record tag
	m_cipher_ctx.gcm_finalize(buf.header(s_tag_size)); 
	//hexdump(stderr, buf.buf(), buf.size()); 
}

bool block_file::simple_dec(uint64_t iv, const slice_t& buf) 
{
	//syslog(LOG_DEBUG, "Doing simple_dec, iv = %ju", iv);
	//hexdump(stderr, buf.buf(), buf.size()); 
	// Set up decryption with proper IV
	m_cipher_ctx.gcm_set_iv(iv);
	// Decrypt in place
	m_cipher_ctx.gcm_partial_decrypt(buf.hrest(s_tag_size));
	// Compute tag
	slice_t tag(s_tag_size);
	m_cipher_ctx.gcm_finalize(tag);
	//hexdump(stderr, buf.buf(), buf.size()); 
	//hexdump(stderr, tag.buf(), tag.size()); 
	// Verify
	if (tag != buf.header(s_tag_size)) {
		syslog(LOG_ERR, "Tag is invalid when reading block");
		return false;
	}
	return true;
}
