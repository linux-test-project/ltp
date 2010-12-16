/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test that implementations may require the requesting process to
 * have the appropriate privilege to set its own scheduling parameters or those
 * of another process.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test that implementations may require the requesting process to\nhave the appropriate privilege to set its own scheduling parameters or those\nof another process.\n");
        return PTS_UNTESTED;
}