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

#include "utils.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

bool write_fully(int fd, const char* buf, int size)
{
	while (size) {
		int r = write(fd, buf, size);
		if (r <= 0) {
			if (errno == EAGAIN) {
				continue;
			}
			return false;
		}
		size -= r;
		buf += r;
	}	
	return true;
}

bool read_fully(int fd, char* buf, int size)
{
	while (size) {
		int r = read(fd, buf, size);
		if (r <= 0) {
			if (errno == EAGAIN) {
				continue;
			}
			return false;
		}
		size -= r;
		buf += r;
	}	
	return true;
}


