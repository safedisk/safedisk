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
#include "block_file.h"
#include "fast_bit.h"

class block_map
{
public:
	block_map(const cipher_key_t& key, uint32_t logical_size);

	bool open(const string& dir);
	bool write(uint32_t logical, const rslice_t& data);
	bool read(uint32_t logical, rslice_t& data_out);
	uint32_t block_count() { return m_logical_size; }
	
private:
	uint64_t phys_expand(uint32_t small);
	uint32_t phys_contract(uint64_t large);
	bool clean_one();

private:	
	const uint32_t s_invalid = -1;
	typedef vector<uint32_t> map_vec_t;
	uint32_t   m_logical_size;
	uint32_t   m_physical_size;
	block_file m_file;
	map_vec_t  m_physical;
	fast_bit   m_in_use;
};
