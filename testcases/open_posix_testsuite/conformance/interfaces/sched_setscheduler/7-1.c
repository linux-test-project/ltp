/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test if implementation-defined restrictions apply as to the
 * appropriate privileges required to set a process' own scheduling policy, or
 * another process' scheduling policy, to a particular value.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test if implementation-defined restrictions apply as to the\nappropriate privileges required to set a process' own scheduling policy, or\nanother process' scheduling policy, to a particular value.\n");
        return PTS_UNTESTED;
}