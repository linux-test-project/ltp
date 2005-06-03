/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_open() function sets errno =  ENOSPC if there is
 * insufficient space for the creation of the new shared memory object.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test that the shm_open() function sets errno to ENOSPC if there is\ninsufficient space for the creation of the new shared memory object.\n");
        return PTS_UNTESTED;
}
