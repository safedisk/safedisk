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

#include "fast_bit.h"
#include <memory.h>

fast_bit::fast_bit(size_t size)
	: m_size(size)
	, m_bits((2 * size + 7) / 8)
{
}

void fast_bit::set(size_t i, bool value) 
{
	i += m_size;
	set_bit(i, value);
	while (i) {
		i /= 2;
		set_bit(i, get_bit(i*2) | get_bit(i*2 + 1));
	}
}

bool fast_bit::get(size_t i)
{
	return get_bit(i + m_size);
}

// Find the first set bit >= start, wrapping if needed
// If there are *no* set bits, returns one past end
// Always doable in log(n) time
size_t fast_bit::find_set(size_t start)
{
	if (get_bit(1) == 0) {  // If top bit isn't set, no bits are set
		return m_size;  // Failure case
	}
	// Otherwise, start at start (corrected for rest of tree)
	size_t i = start + m_size;
	// While we haven't found a set bits
	while (!get_bit(i)) {
		if ((i & (i + 1)) == 0) {  // This is true on 'right edge' of tree
			i = 1;  // This means I hit the end, 'wrap', by going to 1
		} else if (i & 1) { // i is odd
			i++; // Go forward on same level
		} else {  // I is even
			i /= 2;  // Go up one level & forward
		}
	}
	// Loop must have terminated (maybe on 1), thus we need to go down until valid
	while (i < m_size) {
		i *= 2;  // Go down and left
		if (!get_bit(i)) {
			i++;  // If it's not a 1, neighbor must be
		}
	}
	// Convert result back
	return i - m_size;
}

void fast_bit::set_bit(size_t elem, bool value) {
	uint8_t mask = 1 << (elem % 8);
	m_bits[elem / 8] &= ~mask;
	if (value) {
		m_bits[elem / 8] |= mask;
	}
}

bool fast_bit::get_bit(size_t elem) {
	uint8_t mask = 1 << (elem % 8);
	return ((m_bits[elem / 8] & mask) != 0);
}
