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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>

const static uint64_t block_size = 1024;
static void* bm = NULL;
static uint64_t file_size = 0;
static uid_t uid = 0;
static int shutdown = 0;
static int open_count = 0;

#ifdef __APPLE__
static struct timespec create_time;
static struct timespec modify_time;
static struct timespec access_time;
#else
static time_t create_time;
static time_t modify_time;
static time_t access_time;
#endif

extern void* create_block_map(const char* dir, uint32_t blocks, const char* key);
extern void* open_block_map(const char* dir, const char* key);
extern void close_block_map(void* bm);
extern int64_t size_block_map(void* bm);
extern int read_block_map(void* bm, uint32_t block, char* buf);
extern int write_block_map(void* bm, uint32_t block, const char* buf);

static 
int safedisk_getattr(const char* path, struct stat* st)
{
	memset(st, 0, sizeof(*st));

	if (strcmp(path, "/") != 0 && strcmp(path, "/data") != 0) {
		return -ENOENT;
	}
	// Common bits
	st->st_uid = uid;
#ifdef __APPLE__
	st->st_atimespec = access_time;
	st->st_mtimespec = modify_time;
	st->st_ctimespec = create_time;
	st->st_birthtimespec = create_time;
#else
	st->st_atime = access_time;
	st->st_mtime = modify_time;
	st->st_ctime = create_time;
#endif
	st->st_blksize = block_size;
	if (strcmp(path, "/") == 0) { 
		// The root directory of our file system
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 3;
	} 
	else if (strcmp(path, "/data") == 0) { 
		// The data file
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = file_size;
		st->st_blocks = file_size / 512;
	} 

	return 0;
}

int safedisk_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	if (strcmp(path, "/shutdown") == 0) {
		shutdown = 1;
		if (open_count == 0) {
			raise(SIGHUP);
		}
	}
	return -EPERM;
}


static 
int safedisk_open(const char* path, struct fuse_file_info* fi)
{
	if (strcmp(path, "/data") != 0) {
		// We only recognize one file
		return -ENOENT;
	}
	open_count++;	

	return 0;
}

static 
int safedisk_opendir(const char* path, struct fuse_file_info* fi)
{
	open_count++;	
	return 0;
}

static 
int safedisk_release(const char* path, struct fuse_file_info* fi)
{
	open_count--;
	if (open_count == 0 && shutdown) {
		raise(SIGHUP);
	}
	return 0;
}

static 
int safedisk_releasedir(const char* path, struct fuse_file_info* fi)
{
	open_count--;
	if (open_count == 0 && shutdown) {
		raise(SIGHUP);
	}
	return 0;
}

static 
int safedisk_readdir(
	const char*path, 
	void *buf, 
	fuse_fill_dir_t filler,
	off_t offset, 
	struct fuse_file_info* fi)
{
	if (strcmp(path, "/") != 0) {
		// We only recognize the root directory
		return -ENOENT;
	}

	filler(buf, ".", NULL, 0); // Current directory (.)
	filler(buf, "..", NULL, 0); // Parent directory (..) 
	filler(buf, "data", NULL, 0); // The only file we have.

	return 0;
}

static 
int safedisk_read(
	const char* path, 
	char* buf, 
	size_t size, 
	off_t offset,
	struct fuse_file_info* fi)
{
	if (strcmp(path, "/data") != 0) {
		// We can only be reading from the data file
		return -ENOENT;
	}
	
	if (offset > file_size) {
		// Trying to read past the end of file
		return 0;
	}

	if (offset + size > file_size) {
		// Trim the read to the file size
		size = file_size - offset;
	}

	if (offset % block_size != 0 || size % block_size != 0) {
		// Cause non-aligned access to be errors
		return -EIO;
	}
	// Update time
#ifdef __APPLE__
	access_time.tv_sec = time(0);
#else
	access_time = time(0);
#endif
	// Handle everything in blocks	
	size /= block_size;
	offset /= block_size;
	uint32_t block;
	for (block = 0; block < size; block++) {
		if (!read_block_map(bm, offset + block, ((char*) buf) + block * block_size)) {
			return -EIO;
		}
	}

	return size*block_size;
}

static 
int safedisk_write(
	const char* path, 
	const char* buf, 
	size_t size, 
	off_t offset,
	struct fuse_file_info* fi)
{
	if (strcmp(path, "/data") != 0) {
		// We can only be writing to the data file
		return -ENOENT;
	}
	
	if (offset > file_size) {
		// Trying to read past the end of file
		return 0;
	}

	if (offset + size > file_size) {
		// Trim the read to the file size
		size = file_size - offset;
	}

	if (offset % block_size != 0 || size % block_size != 0) {
		// Cause non-aligned access to be errors
		return -EIO;
	}
	// Update time
#ifdef __APPLE__
	modify_time.tv_sec = time(0);
	access_time.tv_sec = time(0);
#else
	modify_time = time(0);
	access_time = time(0);
#endif

	// Handle everything in blocks	
	size /= block_size;
	offset /= block_size;
	uint32_t block;
	for (block = 0; block < size; block++) {
		if (!write_block_map(bm, offset + block, ((const char*) buf) + block * block_size)) {
			return -EIO;
		}
	}

	return size*block_size;
}

static 
struct fuse_operations safedisk_filesystem_operations = {
	.getattr    = safedisk_getattr,    // To provide size, permissions, etc.
	.create     = safedisk_create,     // Allow creation attempts to hit shutdown switch 
	.open       = safedisk_open,       // Allow opening of a single file
	.opendir    = safedisk_opendir,    // Track number of open dirs
	.release    = safedisk_release,    // After close, shutdown
	.releasedir = safedisk_releasedir, // Track number of open dirs
	.read       = safedisk_read,       // Allow block reads
	.write      = safedisk_write,      // Allow block writes
	.readdir    = safedisk_readdir,    // Directory listing of our one directory
};

// TODO: Less lame option parsing 
int main(int argc, char** argv)
{
	openlog("safediskd", LOG_PID | LOG_PERROR, LOG_DAEMON);
	
	// Validate one extra arguments are there
	if (argc < 3) {
		fprintf(stderr, "usage: %s [fuse-options] <mnt_point> <block_dir> [<size>]\n", argv[0]);
		exit(1);
	}
	// Get size if present
	uint32_t size = 0;
	if (argc == 4) {
		size = atoi(argv[--argc]);
		if (size == 0 || size > 1000000) {
			fprintf(stderr, "Size must be non-zero and less than 1 million\n");
			exit(1);
		}
	}
	// Get block_dir
	const char* block_dir = argv[--argc];

	// Ask for password
	char* pass = getpass("Password: ");

	if (size) {
		// If size is set, 'create'
		bm = create_block_map(block_dir, size*1024, pass);
	} else {
		// Otherwise, 'open'
		bm = open_block_map(block_dir, pass);
	}
	if (bm == NULL) {
		fprintf(stderr, "Failed to %s block_map directory\n", size ? "create" : "open");
		exit(1);
	}

	// Check for existance of data directory and get uid
	struct stat st;
	if (stat(block_dir, &st) != 0) {
		fprintf(stderr, "Unable to stat data directory: %s\n", strerror(errno));
		close_block_map(bm);
		exit(1);
	}

	printf("mnt_point: %s\n", argv[argc-1]);
	printf("block_dir: %s\n", block_dir);
	printf("size: %llu\n", size_block_map(bm));

	// Set global variables
	file_size = size_block_map(bm);
	uid = st.st_uid;
#ifdef __APPLE__
	memset(&create_time, 0, sizeof(struct timespec));
	memset(&modify_time, 0, sizeof(struct timespec));
	memset(&access_time, 0, sizeof(struct timespec));
	create_time.tv_sec = time(0);
	modify_time.tv_sec = time(0);
	access_time.tv_sec = time(0);
#else
	create_time = time(0);
	modify_time = time(0);
	access_time = time(0);
#endif
	// Kick off main
	int retcode = fuse_main(argc, argv, &safedisk_filesystem_operations, NULL);
	close_block_map(bm);
	return retcode;
}
