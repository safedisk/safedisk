#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <fuse.h>
#include <stdio.h>

const static uint64_t block_size = 1024;
static void* bm = NULL;
static uint64_t file_size = 0;

extern void* open_block_map(const char* dir, uint32_t blocks, const char* key);
extern void close_block_map(void* bm);
extern int64_t size_block_map(void* bm);
extern int read_block_map(void* bm, uint32_t block, char* buf);
extern int write_block_map(void* bm, uint32_t block, const char* buf);

static int
safedisk_getattr(const char *path, struct stat *stbuf)
{
	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 3;
	} else if (strcmp(path, "/data") == 0) { /* The only file we have. */
		stbuf->st_mode = S_IFREG | 0644;
		stbuf->st_nlink = 1;
		stbuf->st_size = file_size;
		stbuf->st_uid = 501;
		stbuf->st_blksize = 1024;
	} else /* We reject everything else. */
		return -ENOENT;

	return 0;
}

static int
safedisk_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, "/data") != 0) /* We only recognize one file. */
		return -ENOENT;

	return 0;
}

static int
safedisk_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			  off_t offset, struct fuse_file_info *fi)
{
	if (strcmp(path, "/") != 0) /* We only recognize the root directory. */
		return -ENOENT;

	filler(buf, ".", NULL, 0); /* Current directory (.)  */
	filler(buf, "..", NULL, 0); /* Parent directory (..)  */
	filler(buf, "data", NULL, 0); /* The only file we have. */

	return 0;
}

static int
safedisk_read(const char *path, char *buf, size_t size, off_t offset,
		   struct fuse_file_info *fi)
{
	if (strcmp(path, "/data") != 0)
		return -ENOENT;
	
	if (offset > file_size) /* Trying to read past the end of file. */
		return 0;	

	if (offset + size > file_size) /* Trim the read to the file size. */
		size = file_size - offset;

	if (offset % block_size != 0 || size % block_size != 0) /* Cause non-aligned access to be errors */
		return -EIO;
	
	size /= block_size;
	offset /= block_size;
	uint32_t block;
	for(block = 0; block < size; block++) {
		if (!read_block_map(bm, offset + block, ((char*) buf) + block * block_size)) {
			return -EIO;
		}
	}

	return size*block_size;
}

static int
safedisk_write(const char *path, const char *buf, size_t size, off_t offset,
		   struct fuse_file_info *fi)
{
	if (strcmp(path, "/data") != 0)
		return -ENOENT;
	
	if (offset > file_size) /* Trying to read past the end of file. */
		return 0;	

	if (offset + size > file_size) /* Trim the read to the file size. */
		size = file_size - offset;

	if (offset % block_size != 0 || size % block_size != 0) /* Cause non-aligned access to be errors */
		return -EIO;

	size /= block_size;
	offset /= block_size;
	uint32_t block;
	for(block = 0; block < size; block++) {
		if (!write_block_map(bm, offset + block, ((const char*) buf) + block * block_size)) {
			return -EIO;
		}
	}

	return size*block_size;
}

static struct fuse_operations safedisk_filesystem_operations = {
	.getattr = safedisk_getattr, /* To provide size, permissions, etc. */
	.open    = safedisk_open,	/* To enforce read-only access.	   */
	.read    = safedisk_read,	/* To provide file content.		   */
	.write   = safedisk_write,	/* To provide file content.		   */
	.readdir = safedisk_readdir, /* To provide directory listing.	  */
};

int
main(int argc, char **argv)
{
	bm = open_block_map("/tmp/blockmap", 1024*1024, "HelloWorld");
	if (bm == NULL) {
		printf("Failed to make it happen\n");
		return -1;
	}
	file_size = size_block_map(bm);
	int r = fuse_main(argc, argv, &safedisk_filesystem_operations, NULL);
	close_block_map(bm);
	return r;
}

