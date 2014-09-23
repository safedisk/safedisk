#include "block_map.h"
#include <assert.h>

class check_block_map 
{
public:
	check_block_map(uint32_t size, const string& dir)
		: m_size(size)
		, m_dir(dir)
		, m_key(slice_t("HelloWorldHelloWorldHelloWorld12"))
	{
		m_block_map = make_unique<block_map>(m_key, size);
		assert(m_block_map->open(dir));
	}

	void write(uint32_t logical) 
	{
		//printf("Writing to %d\n", (int) logical);
		slice_t data(s_bytes_per_block);
		for (size_t i = 0; i < s_bytes_per_block; i++) {
			data[i] = random();
		}
		
		assert(m_block_map->write(logical, data));
		m_check[logical] = data;
	}

	void read(uint32_t logical) 
	{
		//printf("Reading from %d\n", (int) logical);
		auto it = m_check.find(logical);
		rslice_t check;
		if (it == m_check.end()) {
			slice_t empty(s_bytes_per_block);
			memset(empty.buf(), 0, s_bytes_per_block);
			check = empty;
		} 
		else {
			check = it->second;
		}
		rslice_t proper;
		assert(m_block_map->read(logical, proper));
		assert(proper == check);
	}

	void bounce() {
		m_block_map.reset();
		m_block_map = make_unique<block_map>(m_key, m_size);
		assert(m_block_map->open(m_dir));
	}

private:
	size_t m_size;
	string m_dir;
	cipher_key_t m_key;
	std::map<uint32_t, rslice_t> m_check;
	unique_ptr<block_map> m_block_map;
};

void test_block_map()
{
	int retcode = system("rm -rf /tmp/test_block_map");
	assert(!retcode);
	retcode = system("mkdir /tmp/test_block_map");
	assert(!retcode);
	size_t size = 333;
	check_block_map cbm(size, "/tmp/test_block_map");
	for (size_t i = 0; i < 100000; i++) {
		cbm.write(random() % size);
		cbm.read(random() % size);
		if (random() % 100 == 0) {
			cbm.bounce();
		}	
	}
}
