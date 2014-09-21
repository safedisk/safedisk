
#include "block_map.h"
#include "digest.h"

extern "C" void* open_block_map(const char* dir, uint32_t blocks, const char* key) 
{
	cipher_key_t k = compute_digest(slice_t(key)).cast();
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

