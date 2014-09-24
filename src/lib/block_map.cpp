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

#include <syslog.h>

block_map::block_map(const cipher_key_t& key, uint32_t logical_size) 
	: m_logical_size(logical_size)
	, m_physical_size(2*logical_size)
	, m_file(key)
	, m_physical(m_logical_size)
	, m_in_use(m_physical_size)
{}

bool block_map::open(const string& dir)
{
	if (!m_file.open(dir)) {
		return false;
	}
	std::fill(m_physical.begin(), m_physical.end(), s_invalid);
	bool r = m_file.scan([&](uint64_t phys, uint32_t logical) {
		//syslog(LOG_DEBUG, "Read mapping: %llu -> %u", phys, logical);
		assert(logical < m_logical_size);
		uint32_t phys_small = phys_contract(phys);
		uint32_t old = m_physical[logical];
		if (old != s_invalid) {
			m_in_use.set(old, false);
		}
		m_in_use.set(phys_small, true);
		m_physical[logical] = phys_small;	
	});
	return r;
}

bool block_map::write(uint32_t logical, const rslice_t& data)
{
	// Free old physical block for this logical block (if any)
	uint32_t prev = m_physical[logical];
	if (prev != s_invalid) {
		// Remove old in-use
		m_in_use.set(prev, false);
		// Move one block forward
		if (!clean_one()) {
			return false;
		}
	}
	// Do write
	uint64_t phys;
	if (!m_file.write_block(logical, data, phys)) {
		return false;
	}
	// Update mappings
	m_in_use.set(phys_contract(phys), true);
	m_physical[logical] = phys_contract(phys);	
	// Do 'erase'
	if (m_file.top() > m_physical_size) {
		if (!m_file.remove_old(m_file.top() - m_physical_size)) {
			return false;
		}
	}
	return true;
}

bool block_map::read(uint32_t logical, rslice_t& data_out)
{
	// Look up physical address
	uint32_t phys_small = m_physical[logical];
	// If it's empty, return all 0's
	if (phys_small == s_invalid) {
		//syslog(LOG_DEBUG, "Returning empty");
		slice_t empty(s_bytes_per_block);
		memset(empty.buf(), 0, s_bytes_per_block);	
		data_out = empty;
		return true;
	}
	// Otherwise get the real data
	uint32_t logical2;
	bool r = m_file.read_block(phys_expand(phys_small), data_out, logical2);
	// Return success code 
	return r && logical == logical2;
}

bool block_map::clean_one()
{
	// Start by finding oldest block
	uint32_t in_use = m_in_use.find_set(phys_contract(m_file.top()));
	// If nothing is in use, return with success
	if (in_use == m_physical_size) {
		return true;
	}
	// Read the block in, determine where it's logical location is	
	uint64_t phys = phys_expand(in_use);
	uint32_t logical = 0;
	rslice_t block;
	if (!m_file.read_block(phys, block, logical)) {
		return false;
	}
	// Remove it from use
	m_in_use.set(in_use, false);
	// Rewrite
	if (!m_file.write_block(logical, block, phys)) {
		return false;
	}
	// Update mappings
	m_in_use.set(phys_contract(phys), true);
	m_physical[logical] = phys_contract(phys);	
	return true;
}

uint64_t block_map::phys_expand(uint32_t small) {
	uint64_t m_fwd_steps = m_file.top() / m_physical_size;
	uint64_t phys = m_fwd_steps * m_physical_size + uint64_t(small);
	if (phys >= m_file.top()) phys -= m_physical_size;
	return phys;
}

uint32_t block_map::phys_contract(uint64_t large) {
	return large % m_physical_size;
}
