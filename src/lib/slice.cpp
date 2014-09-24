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

#include "slice.h"
#include <assert.h>

struct slice_t::buffer
{
	uint32_t ref_count;
	uint32_t capacity; 
	char*    buf;
};

slice_t::slice_t(const char* buf)
	: m_base(new buffer)
	, m_size(strlen(buf))
{
	m_base->ref_count = 1;
	m_base->capacity = m_size;
	m_base->buf = new char[m_base->capacity];
	memcpy(m_base->buf, buf, m_size);
}

slice_t::slice_t(const char* buf, uint32_t size)
	: m_base(new buffer)
	, m_size(size)
{
	m_base->ref_count = 1;
	m_base->capacity = m_size;
	m_base->buf = new char[m_base->capacity];
	memcpy(m_base->buf, buf, m_size);
}

slice_t::slice_t(const rslice_t& rhs)
	: slice_t(rhs.buf(), rhs.size())
{}

slice_t::slice_t(uint32_t size, uint32_t tail)
	: m_base(new buffer)
	, m_size(size)
{
	m_base->ref_count = 1;
	m_base->capacity = size + tail;
	m_base->buf = new char[m_base->capacity];		
	memset(m_base->buf, 0, m_size);
}

slice_t::slice_t(const slice_t& rhs)
	: m_base(rhs.m_base)
	, m_offset(rhs.m_offset)
	, m_size(rhs.m_size)
{
	if (m_base) {
		m_base->ref_count++;
	}
}

slice_t::slice_t(slice_t&& rhs)
	: m_base(rhs.m_base)
	, m_offset(rhs.m_offset)
	, m_size(rhs.m_size)
{
	rhs.m_base = nullptr;
	rhs.m_offset = 0;
	rhs.m_size = 0;
}

slice_t& slice_t::operator=(slice_t rhs)
{
	swap(m_base, rhs.m_base);
	swap(m_offset, rhs.m_offset);
	swap(m_size, rhs.m_size);
	return *this;
}

slice_t::~slice_t()
{
	if (!m_base) {
		return;
	}
	m_base->ref_count--;
	if (m_base->ref_count == 0) {
		delete[] m_base->buf;
		delete m_base;
	}
}

char& slice_t::operator[](uint32_t x) const
{ 
	return m_base->buf[m_offset + x];
}

static uint32_t nearest_power_of_two(uint32_t size)
{
	uint32_t r = 1;
	while (r < size) {
		r <<= 1;
	}
	return r;
}

void slice_t::add_tail(uint32_t size)
{
	if (m_base && m_base->ref_count == 1 && 
		m_base->capacity - (m_offset + m_size) >= size) {
		m_size += size;
	} else {	
		uint32_t newsize = nearest_power_of_two(size + m_size);
		slice_t r(newsize);
		if (m_size > 0) {
			memcpy(r.m_base->buf, m_base->buf + m_offset, m_size);
		}
		r.m_offset = 0;
		r.m_size = size + m_size;
		swap(m_base, r.m_base);
		swap(m_offset, r.m_offset);
		swap(m_size, r.m_size);
	}
}

void slice_t::push_back(char c) 
{
	uint32_t off = size();
	add_tail(1);
	*(buf() + off) = c;
}

void slice_t::append(const char* _buf, uint32_t len) 
{
	uint32_t off = size();
	add_tail(len);
	memcpy(buf() + off, _buf, len);
}

void slice_t::append(const rslice_t& s) 
{
	append(s.buf(), s.size());
}

slice_t slice_t::slice(uint32_t offset, uint32_t size) const
{
	assert(offset + size <= m_size);
	slice_t r = *this;
	r.m_offset += offset;
	r.m_size = size;
	return r;
}

char* slice_t::buf() const
{
	assert(m_base);
	return m_base->buf + m_offset;
}

byte* slice_t::ubuf() const
{
	assert(m_base);
	return (byte *) (m_base->buf + m_offset);
}

uint32_t slice_t::size() const
{
	return m_size;
}

bool slice_t::operator<(const rslice_t& rhs) const 
{
	uint32_t overlap = std::min(size(), rhs.size());
	int mcr = memcmp(buf(), rhs.buf(), overlap);
	if (mcr < 0) {
		return true;
	}
	if (mcr > 0) {
		return false;
	}
	if (size() < rhs.size()) {
		return true;
	}
	return false;
}

bool slice_t::operator==(const rslice_t& rhs) const 
{
	if (size() != rhs.size()) {
		return false;
	}
	return memcmp(buf(), rhs.buf(), size()) == 0;
}

rslice_t::rslice_t(const slice_t& rhs)
	: m_base(rhs.m_base)
	, m_offset(rhs.m_offset)
	, m_size(rhs.m_size)
{
	if (m_base) {
		m_base->ref_count++;
	}
}

rslice_t::rslice_t(const rslice_t& rhs)
	: m_base(rhs.m_base)
	, m_offset(rhs.m_offset)
	, m_size(rhs.m_size)
{
	if (m_base) {
		m_base->ref_count++;
	}
}

rslice_t::rslice_t(rslice_t&& rhs)
	: m_base(rhs.m_base)
	, m_offset(rhs.m_offset)
	, m_size(rhs.m_size)
{
	rhs.m_base = nullptr;
	rhs.m_offset = 0;
	rhs.m_size = 0;
}

rslice_t& rslice_t::operator=(rslice_t rhs)
{
	swap(m_base, rhs.m_base);
	swap(m_offset, rhs.m_offset);
	swap(m_size, rhs.m_size);
	return *this;
}

rslice_t::~rslice_t()
{
	if (!m_base) {
		return;
	}
	m_base->ref_count--;
	if (m_base->ref_count == 0) {
		delete[] m_base->buf;
		delete m_base;
	}
}

char rslice_t::operator[](uint32_t x) const
{
	return m_base->buf[m_offset + x];
}

rslice_t rslice_t::slice(uint32_t offset, uint32_t size) const
{
	assert(offset + size <= m_size);
	rslice_t r = *this;
	r.m_offset += offset;
	r.m_size = size;
	return r;
}

const char* rslice_t::buf() const
{
	assert(m_base);
	return m_base->buf + m_offset;
}

const byte* rslice_t::ubuf() const
{
	assert(m_base);
	return (byte *) (m_base->buf + m_offset);
}

uint32_t rslice_t::size() const
{
	return m_size;
}

bool rslice_t::operator<(const rslice_t& rhs) const 
{
	uint32_t overlap = std::min(size(), rhs.size());
	int mcr = memcmp(buf(), rhs.buf(), overlap);
	if (mcr < 0) {
		return true;
	}
	if (mcr > 0) {
		return false;
	}
	if (size() < rhs.size()) {
		return true;
	}
	return false;
}

bool rslice_t::operator==(const rslice_t& rhs) const 
{
	if (size() != rhs.size()) {
		return false;
	}
	return memcmp(buf(), rhs.buf(), size()) == 0;
}
