/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test the result of shm_open() when using O_TRUNC with O_RDONLY. 
 * It is undefined.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test the result of shm_open() when using O_TRUNC with O_RDONLY.\nIt is undefined.\n");
        return PTS_UNTESTED;
}

