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
        printf("Will not test that the shm_open() function create an open file description\nthat refers to the shared memory object and a file descriptor that refers to\nthat open file description.");
        return PTS_UNTESTED;
}

