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

class fast_bit 
{
public:
	fast_bit(size_t size);
	
	void set(size_t elem, bool value);
	bool get(size_t elem);
	size_t find_set(size_t start);
	
private:
	void set_bit(size_t elem, bool value);
	bool get_bit(size_t elem);

private:
	// Size of actual bits
	size_t m_size;

	// Includes main bits + rollup bits
	std::vector<uint8_t> m_bits;
};
