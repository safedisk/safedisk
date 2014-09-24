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

