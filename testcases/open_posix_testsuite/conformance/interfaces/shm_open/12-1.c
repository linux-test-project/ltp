/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test the behavior of implementation when an application does not
 * specify exactly one of the first two values (access modes) below in the
 * value of oflag:
 *     O_RDONLY
 *     O_RDWR
 *
 * Often O_RDONLY == 0 and O_RDWR == 2^n, so when both values are specified,
 * the result is the same than when only O_RDWR is set.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test the behavior of implementation when an application does not\nspecify exactly one of two values: O_RDONLY and O_RDWR.\n");
        return PTS_UNTESTED;
}