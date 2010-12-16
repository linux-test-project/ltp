/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test that implementations may require that the requesting process
 * have permission to set its own scheduling parameters or those of another
 * process.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test that implementations may require that the requesting process\nhave permission to set its own scheduling parameters or those of another\nprocess.\n");
        return PTS_UNTESTED;
}