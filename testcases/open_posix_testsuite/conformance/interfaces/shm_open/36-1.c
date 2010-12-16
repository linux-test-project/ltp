/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_open() function sets errno = EINTR if the operation was
 * interrupted by a signal.
 *
 * This is very difficult to test for a non blocking function.
 */

#include <stdio.h>
#include "posixtest.h"

int main() {
	printf("It is very difficult to test that the shm_open() function sets errno = EINTR\nwhen it is interrupted by a signal.\n");
	return PTS_UNTESTED;
}