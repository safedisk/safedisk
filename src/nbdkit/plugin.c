
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <nbdkit-plugin.h>

static const uint64_t block_size = 1024;

extern void* open_block_map(const char* dir, uint32_t blocks, const char* key);
extern void close_block_map(void* bm);
extern int64_t size_block_map(void* bm);
extern int read_block_map(void* bm, uint32_t block, char* buf);
extern int write_block_map(void* bm, uint32_t block, const char* buf);

#define THREAD_MODEL NBDKIT_THREAD_MODEL_SERIALIZE_ALL_REQUESTS

static int x;

static void* safedisk_open(int readonly)
{
	nbdkit_debug("In open\n");
	return open_block_map("/tmp/wtf", 1000, "HelloWorld");
}

static void safedisk_close(void *handle)
{
	nbdkit_debug("In close\n");
	close_block_map(handle);	
}

static int64_t safedisk_get_size(void *handle)
{
	nbdkit_debug("In get_size\n");
	return size_block_map(handle);	
}

static int safedisk_pread(void *handle, void *buf, uint32_t count, uint64_t offset)
{
	nbdkit_debug("In pread\n");
	nbdkit_debug("count = %d, offset = %d\n", count, (int) offset);
	assert(count % block_size == 0);
	assert(offset % block_size == 0);
	count /= block_size;
	offset /= block_size;
	uint32_t block;
	for(block = 0; block < count; block++) {
		if (!read_block_map(handle, offset + block, ((char*) buf) + block * block_size)) {
			return -1;
		}
	}
	nbdkit_debug("Done\n");
	return 0;
}

static int safedisk_pwrite(void *handle, const void *buf, uint32_t count, uint64_t offset)
{
	nbdkit_debug("In pwrite\n");
	nbdkit_debug("count = %d, offset = %d\n", count, (int) offset);
	assert(count % block_size == 0);
	assert(offset % block_size == 0);
	count /= block_size;
	offset /= block_size;
	uint32_t block;
	for(block = 0; block < count; block++) {
		if (!write_block_map(handle, offset + block, ((const char*) buf) + block * block_size)) {
			return -1;
		}
	}
	nbdkit_debug("Done\n");
	return 0;
}

static struct nbdkit_plugin plugin = {
   .name              = "safedisk",
   .version           = "0.0.1",
   .longname          = "safedisk",
   .description       = "Full disk encryption with backup",
   .config_help       = "Some help",
   .open              = safedisk_open,
   .close             = safedisk_close,
   .get_size          = safedisk_get_size,
   .pread             = safedisk_pread,
   .pwrite            = safedisk_pwrite,
};

NBDKIT_REGISTER_PLUGIN(plugin)

