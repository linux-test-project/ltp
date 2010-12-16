/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test whether the name appears in the file system and is visible to
 * other functions that take pathnames as arguments because it is unspecified.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test whether the name appears in the file system and is visible to\nother functions that take pathnames as arguments because it is unspecified.\n");
        return PTS_UNTESTED;
}