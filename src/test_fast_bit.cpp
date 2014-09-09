#include "fast_bit.h"

class slow_bit 
{
public:
	slow_bit(size_t size) 
		: m_bits(size) 
	{}

	void set(size_t elem, bool value) 
	{ 
		m_bits[elem] = value; 
	}

	bool get(size_t elem) 
	{
		return m_bits[elem]; 
	}

	size_t find_set(size_t start) 
	{
		size_t i = start;
		do {
			if (m_bits[i]) {
				return i;
			}
			i++;
			i %= m_bits.size();
		} while (i != start);
		return m_bits.size();
	}

private:
	vector<bool> m_bits;
};

void random_test(size_t size, size_t count) 
{
	slow_bit sb(size);
	fast_bit fb(size);
	for (size_t i = 0; i < count; i++) {
		size_t x = random() % size;
		sb.set(x, true);
		fb.set(x, true);
	}
	for (size_t i = 0; i < count; i++) {
		size_t x = random() % size;
		assert(sb.get(x) == fb.get(x));
		size_t xs = sb.find_set(x);
		size_t xf = fb.find_set(x);
		assert(xs == xf);
		sb.set(xs, false);
		fb.set(xf, false);
	}
	assert(sb.find_set(size / 2) == size);
	assert(fb.find_set(size / 2) == size);
}

void test_fast_bit()
{
	printf("Doing test of fast_bit\n");
	for (size_t c = 0; c < 100; c++) {
		random_test(1001, 703);
	}
	printf("fast_bit worked!\n");
}
