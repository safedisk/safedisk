#include <stdio.h>
#include <syslog.h>

void test_fast_bit();
void test_block_map();

int main()
{	
	openlog("safedisk", LOG_PERROR, LOG_DAEMON);
	printf("Hello world\n");
	test_fast_bit();
	// Before running test_block_map, it's probably a good idea to change
	// File size params in block_file.h to hit the edge cases, and prevent the tests
	// from taking forever
	//test_block_map();
	return 0;
}
