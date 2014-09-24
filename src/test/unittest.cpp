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
