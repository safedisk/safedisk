/*  Safedisk
 *  Copyright (C) 2014  Jeremy Bruestle
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "block_map.h"
#include "digest.h"
#include <unistd.h>
#include <sys/stat.h>

struct meta_data
{
	uint32_t blocks;
};

/* TODO: Move these two static functions into the logic of block_map directly */
static bool make_meta_file(const string& filename, const cipher_key_t& k, const meta_data& md)
{
	cipher_ctx_t ctx(k);
	slice_t s((char*) &md, sizeof(meta_data));
	uint64_t iv = 0;
	iv--;  // Magic meta-data iv
	slice_t r = ctx.encrypt_and_sign(iv, s);
	FILE *f = fopen(filename.c_str(), "w");
	if (f == NULL) {
		fprintf(stderr, "Unable to make meta-file: %s\n", filename.c_str());
		return false;
	}
	if (fwrite(r.buf(), 1, r.size(), f) != r.size()) {
		fprintf(stderr, "Unable to write meta-file: %s\n", filename.c_str());
		fclose(f);
		unlink(filename.c_str());
		return false;
	}
	fclose(f);
	return true;
}

static bool read_meta_file(const string& filename, const cipher_key_t& k, meta_data& md)
{
	FILE *f = fopen(filename.c_str(), "r");
	if (f == NULL) {
		fprintf(stderr, "Unable to open meta-file: %s\n", filename.c_str());
		return false;
	}
	slice_t s(sizeof(meta_data) + 16);
	if (fread(s.buf(), 1, s.size(), f) != s.size()) {
		fprintf(stderr, "Unable to read meta-file: %s\n", filename.c_str());
		fclose(f);
		return false;
	}
	fclose(f);
	cipher_ctx_t ctx(k);
	uint64_t iv = 0;
	iv--;  // Magic meta-data iv
	slice_t r;
	if (!ctx.decrypt_and_verify(iv, r, s)) {
		fprintf(stderr, "Unable to decrypt meta-file: %s\n", filename.c_str());
		return false;
	}
	memcpy((char*) &md, r.buf(), r.size());
	return true;
}

extern "C" void* create_block_map(const char* dir, uint32_t blocks, const char* key)
{
	cipher_key_t k = compute_digest(slice_t(key)).cast();
	int r = mkdir(dir, 0777);
	if (r < 0) {
		fprintf(stderr, "Unable to make directory %s: %s\n", dir, strerror(errno));
		return NULL;
	}

	meta_data md;
	md.blocks = htonl(blocks);
	if (!make_meta_file(string(dir) + "/meta", k, md)) {
		rmdir(dir);
		return NULL;
	}

	block_map* bm = new block_map(k, blocks);
	if (!bm->open(dir)) {
		delete bm;
		return NULL;
	}
	return bm;
}

extern "C" void* open_block_map(const char* dir, const char* key)
{
	cipher_key_t k = compute_digest(slice_t(key)).cast();

	meta_data md;
	if (!read_meta_file(string(dir) + "/meta", k, md)) {
		rmdir(dir);
		return NULL;
	}
	uint32_t blocks = ntohl(md.blocks);

	block_map* bm = new block_map(k, blocks);
	if (!bm->open(dir)) {
		delete bm;
		return NULL;
	}
	return bm;
}

extern "C" void close_block_map(void* bm)
{
	delete ((block_map*) bm);
}

extern "C" uint64_t size_block_map(void* bm)
{
	return uint64_t(((block_map*) bm)->block_count()) * s_bytes_per_block;
}

extern "C" int read_block_map(void* bm, uint32_t block, char* buf)
{
	// TODO: Make slice stuff support external buffers
	// This is actually pretty easy, but not relevant for now
	rslice_t data;
	bool r = ((block_map*) bm)->read(block, data);
	if (r) {
		memcpy(buf, data.buf(), s_bytes_per_block);
	}
	return r ? 1 : 0;
}

extern "C" int write_block_map(void* bm, uint32_t block, const char* buf)
{
	// TODO: Make slice stuff support external buffers
	// This is actually pretty easy, but not relevant for now
	slice_t data(buf, s_bytes_per_block);
	bool r = ((block_map*) bm)->write(block, data);
	return r ? 1 : 0;
}

