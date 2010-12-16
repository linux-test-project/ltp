/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test that the shm_open() function create an open file description
 * that refers to the shared memory object and a file descriptor that refers to
 * that open file description.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Test doesn't exercise requirements; read code comments for "
		"more details of what isn't tested\n");
        return PTS_UNTESTED;
}