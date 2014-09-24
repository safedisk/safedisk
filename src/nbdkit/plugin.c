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


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <nbdkit-plugin.h>

static const uint64_t block_size = 1024;

extern void* open_block_map(const char* dir, uint32_t blocks, const char* key);
extern void close_block_map(void* bm);
extern int64_t size_block_map(void* bm);
extern int read_block_map(void* bm, uint32_t block, char* buf);
extern int write_block_map(void* bm, uint32_t block, const char* buf);

#define THREAD_MODEL NBDKIT_THREAD_MODEL_SERIALIZE_ALL_REQUESTS

uint32_t size = 0;
static const char* dir = NULL;
static const char* key= NULL;

static int safedisk_config(const char *k, const char *v)
{
	if (strcmp(k, "dir") == 0) {
		dir = v;
	} else if (strcmp(k, "key") == 0) {
		key = v;
	} else if (strcmp(k, "size") == 0) {
		size = atoi(v);
		if (size == 0) {
			nbdkit_error("Invalid size");
			return -1;
		}
	} else {
		nbdkit_error("Unknown config key");
		return -1;
	}
	return 0;
}

static int safedisk_config_complete(void)
{
	if (dir == NULL) {
		nbdkit_error("'dir' parameter required");
		return -1;
	}
	if (key == NULL) {
		nbdkit_error("'key' parameter required");
		return -1;
	}
	if (size == 0) {
		nbdkit_error("'size' parameter required");
		return -1;
	}

	dir = nbdkit_absolute_path(dir);
	if (dir == NULL) {
		return -1;
	}
	mkdir(dir, 0700);
	return 0;
}

static void* safedisk_open(int readonly)
{
	nbdkit_debug("In open\n");
	return open_block_map(dir, size * 1024, key);
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
   .config_help       = "dir=<directory for files> size=<size in MBs> key=<cipher key>",
   .config            = safedisk_config,
   .config_complete   = safedisk_config_complete,
   .open              = safedisk_open,
   .close             = safedisk_close,
   .get_size          = safedisk_get_size,
   .pread             = safedisk_pread,
   .pwrite            = safedisk_pwrite,
};

NBDKIT_REGISTER_PLUGIN(plugin)

