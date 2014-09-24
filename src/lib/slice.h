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

class rslice_t;

class slice_t : comparable<slice_t>
{
	friend class rslice_t;
public:
	// Make a new nil slice
	slice_t() = default;
	// Make a slice from a copy of a simple null terminated string
	slice_t(const char* buf);
	// Make a slice from a copy of a buffer
	slice_t(const char* buf, uint32_t size);
	// Copy an rslice
	slice_t(const rslice_t& rhs);

	// Make a slice of 'size' bytes, with possible additional capacity
	slice_t(uint32_t size, uint32_t tail = 0);

	// Standard c++ memory management goo
	slice_t(const slice_t& rhs);
	slice_t(slice_t&& rhs);
	slice_t& operator=(slice_t rhs);
	~slice_t();

	// Access individual elements
	char& operator[](uint32_t x) const;

	// Expand, makes a copy unless uniquely reference
	void add_tail(uint32_t size);

	// Expand and add some data
	void push_back(char x);
	void append(const char* buf, uint32_t size);
	void append(const rslice_t& s);
	
	// Shrink, always a reference
	slice_t slice(uint32_t offset, uint32_t size) const;
	slice_t header(uint32_t hsize) const { assert(hsize <= size()); return slice(0, hsize); }
	slice_t hrest(uint32_t hsize) const { assert(hsize <= size()); return slice(hsize, size() - hsize); }

	// Get actual buffer (or null_ptr if nil)	
	char* buf() const;
	byte* ubuf() const;
	uint32_t size() const;

	// Comparison operators
	bool operator<(const rslice_t& rhs) const;	
	bool operator==(const rslice_t& rhs) const;	

private:
	struct buffer;
	buffer*  m_base = nullptr;
	uint32_t m_offset = 0;
	uint32_t m_size = 0;
};

class rslice_t : comparable<rslice_t>
{
public:
	// Make an empty read only slice
	rslice_t() = default;
	// Make a read only view of a slice
	rslice_t(const slice_t& rhs);

	// Standard c++ memory management goo
	rslice_t(const rslice_t& rhs);
	rslice_t(rslice_t&& rhs);
	rslice_t& operator=(rslice_t rhs);
	~rslice_t();

	// Access individual elements
	char operator[](uint32_t x) const;

	// Shrink, always a reference
	rslice_t slice(uint32_t offset, uint32_t size) const;
	slice_t header(uint32_t hsize) const { assert(hsize <= size()); return slice(0, hsize); }
	slice_t hrest(uint32_t hsize) const { assert(hsize <= size()); return slice(hsize, size() - hsize); }

	// Get actual buffer (or null_ptr if nil)	
	const char* buf() const;
	const byte* ubuf() const;
	uint32_t size() const;

	// Comparison operators
	bool operator<(const rslice_t& rhs) const;	
	bool operator==(const rslice_t& rhs) const;	

private:
	slice_t::buffer* m_base = nullptr;
	uint32_t m_offset = 0;
	uint32_t m_size = 0;
};

// Use to make 'fixed' size subtypes via CRTP
template<typename derived, size_t csize> 
class fslice_t : comparable<fslice_t<derived, csize> >
{
public:
	// Make an empty (null) version
	fslice_t() = default;

	// Get from a slice
	fslice_t(const slice_t& rhs) : m_inner(rhs) 
	{
		assert(m_inner.size() == csize); 
	}

	// Compare
	bool operator<(const fslice_t& rhs) const { return m_inner < rhs.m_inner; }
	bool operator==(const fslice_t& rhs) const { return m_inner == rhs.m_inner; }
	
	// Convert back 
	const slice_t& cast() const { return m_inner; } 

private:
	slice_t m_inner;
};

// Do the same for rslices
template<typename derived, size_t csize> 
class frslice_t : comparable<frslice_t<derived, csize> >
{
public:
	const static uint32_t required_size = csize;

	// Make an empty (null) version
	frslice_t() = default;

	// Get from a slice
	frslice_t(const slice_t& rhs) : m_inner(rhs) 
	{
		assert(m_inner.size() == csize); 
	}

	// Get from an rslice
	frslice_t(const rslice_t& rhs) : m_inner(rhs) 
	{
		assert(m_inner.size() == csize);
	}

	// Compare
	bool operator<(const frslice_t& rhs) const { return m_inner < rhs.m_inner; }
	bool operator==(const frslice_t& rhs) const { return m_inner == rhs.m_inner; }
	
	// Convert back 
	const rslice_t& cast() const { return m_inner; } 

private:
	rslice_t m_inner;
};

