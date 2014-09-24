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

#pragma once

#include "types.h"
#include "cipher.h"

static const uint64_t s_bytes_per_block = 1024;   // Block size in bytes
static const uint64_t s_blocks_per_region = 1024;  // Region size in blocks
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
	bool next_chunk(uint64_t chunk_id);
	string file_name(uint64_t chunk_id);
	void simple_enc(uint64_t iv, const slice_t& buf);
	bool simple_dec(uint64_t iv, const slice_t& buf);

private:
	struct file_info 
	{
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
};
