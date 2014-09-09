
#include "types.h"
#include "cipher.h"

static const uint64_t s_bytes_per_block = 4096;   // Block size in bytes
static const uint64_t s_blocks_per_region = 4096;  // Region size in blocks
static const uint64_t s_regions_per_chunk = 256;    // Chunk size in regions 

//For tests
/*
static const uint64_t s_bytes_per_block = 50;   // Block size in bytes
static const uint64_t s_blocks_per_region = 5;  // Region size in blocks
static const uint64_t s_regions_per_chunk = 3;    // Chunk size in regions 
*/

class block_file 
{
public:
	// Construct a block file
	block_file(const cipher_key_t& key);
	// Destruct
	~block_file() { close(); }
	// Open a directory, recover any existing blocks
	bool open(const string& dir);
	// Close nicely
	void close();
	// Scan existing block file, call functor with physical->logical mapping, replay in physical order
	bool scan(std::function<void (uint64_t, uint32_t)> callback);
	// Removes old chunks, keeping only physical blocks >= keep_after
	bool remove_old(uint64_t keep_after); 
	// Writes a block, returns true if no errors, also returns physical location
	bool write_block(uint32_t logical, const rslice_t& block, uint64_t& physical_out);
	// Reads a block, return true if no errors
	bool read_block(uint64_t physical, rslice_t& block_out, uint32_t& logical_out);
	// Get 'top' of physical space
	uint64_t top() { return m_next; }
private:
	struct file_info {
		int   fd;
		off_t size;
	};
	typedef map<uint64_t, file_info> chunk_map_t;
	cipher_ctx_t m_cipher_ctx;
	string       m_dir;
	chunk_map_t  m_chunks;
	uint64_t     m_next;
	file_info*   m_fi;
	slice_t      m_region_footer;
	slice_t      m_chunk_footer;

	bool next_chunk(uint64_t chunk_id);
	string file_name(uint64_t chunk_id);
	void simple_enc(uint64_t iv, const slice_t& buf);
	bool simple_dec(uint64_t iv, const slice_t& buf);
};

