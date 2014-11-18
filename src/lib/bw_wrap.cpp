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
#include <arpa/inet.h>
#include <openssl/rand.h>
extern "C" {
#include <libscrypt.h>
};

struct meta_data
{
	uint32_t blocks;
};

static bool make_file(const string& filename, const rslice_t& data) 
{
	FILE *f = fopen(filename.c_str(), "w");
	if (f == NULL) {
		fprintf(stderr, "Unable to make file: %s\n", filename.c_str());
		return false;
	}
	if (fwrite(data.buf(), 1, data.size(), f) != data.size()) {
		fprintf(stderr, "Unable to write file: %s\n", filename.c_str());
		fclose(f);
		unlink(filename.c_str());
		return false;
	}
	fclose(f);
	return true;
}

static bool read_file(const string& filename, slice_t& data)
{
	FILE *f = fopen(filename.c_str(), "r");
	if (f == NULL) {
		fprintf(stderr, "Unable to open file: %s\n", filename.c_str());
		return false;
	}
	if (fread(data.buf(), 1, data.size(), f) != data.size()) {
		fprintf(stderr, "Unable to read file: %s\n", filename.c_str());
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}

/* TODO: Move these two static functions into the logic of block_map directly */
static bool make_meta_file(const string& filename, const cipher_key_t& k, const meta_data& md)
{
	cipher_ctx_t ctx(k);
	slice_t s((char*) &md, sizeof(meta_data));
	uint64_t iv = 0;
	iv--;  // Magic meta-data iv
	slice_t r = ctx.encrypt_and_sign(iv, s);
	return make_file(filename, r);
}

static bool read_meta_file(const string& filename, const cipher_key_t& k, meta_data& md)
{
	slice_t s(sizeof(meta_data) + 16);
	if (!read_file(filename, s)) {
		return false;
	}	
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
	int r = mkdir(dir, 0777);
	if (r < 0) {
		fprintf(stderr, "Unable to make directory %s: %s\n", dir, strerror(errno));
		return NULL;
	}
	slice_t salt(32);
	RAND_pseudo_bytes(salt.ubuf(), salt.size());
	if (!make_file(string(dir) + "/salt", salt)) {
		return NULL;
	}
	slice_t kbuf(32);
	r = libscrypt_scrypt(
		(const unsigned char*) key, strlen(key), 
		salt.ubuf(), salt.size(), 
		SCRYPT_N, SCRYPT_r, SCRYPT_p, 
		kbuf.ubuf(), kbuf.size());
	if (r != 0) {
		unlink((string(dir) + "/salt").c_str());
		rmdir(dir);
		return NULL;
	}

	cipher_key_t k = kbuf;

	meta_data md;
	md.blocks = htonl(blocks);
	if (!make_meta_file(string(dir) + "/meta", k, md)) {
		unlink((string(dir) + "/salt").c_str());
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
	slice_t salt(32);
	if (!read_file(string(dir) + "/salt", salt)) {
		return NULL;
	}
	slice_t kbuf(32);
	int r = libscrypt_scrypt(
		(const unsigned char*) key, strlen(key), 
		salt.ubuf(), salt.size(), 
		SCRYPT_N, SCRYPT_r, SCRYPT_p, 
		kbuf.ubuf(), kbuf.size());
	if (r != 0) {
		return NULL;
	}

	cipher_key_t k = kbuf;

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

