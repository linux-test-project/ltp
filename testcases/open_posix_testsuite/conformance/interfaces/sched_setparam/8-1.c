/* 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * Will not test that the target process is moved to the tail of the thread
 * list for its priority when it is running.
 */

#include <stdio.h>
#include "posixtest.h"

int main() {
	printf("Will not test that the target process is moved to the tail of the thread\nlist for its priority when it is running.\n");
	return PTS_UNTESTED;
}
