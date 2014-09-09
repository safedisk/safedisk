
#include "utils.h"
#include <syslog.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

bool write_fully(int fd, const char* buf, int size)
{
	while(size) {
		int r = write(fd, buf, size);
		if (r <= 0) {
			if (errno == EAGAIN) continue;
			return false;
		}
		size -= r;
		buf += r;
	}	
	return true;
}

bool read_fully(int fd, char* buf, int size)
{
	while(size) {
		int r = read(fd, buf, size);
		if (r <= 0) {
			if (errno == EAGAIN) continue;
			return false;
		}
		size -= r;
		buf += r;
	}	
	return true;
}


