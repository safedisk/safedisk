
#include "types.h"

class fast_bit 
{
public:
	fast_bit(size_t size);
	~fast_bit();
	void set(size_t elem, bool value);
	bool get(size_t elem);
	size_t find_set(size_t start);
private:
	void set_bit(size_t elem, bool value);
	bool get_bit(size_t elem);
	// Size of actual bits
	size_t   m_size;
	// Includes main bits + rollup bits
	uint8_t* m_bits;
};
