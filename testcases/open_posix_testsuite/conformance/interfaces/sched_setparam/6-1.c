/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test the conditions under which one process has permission to
 * change the scheduling parameters of another process, because they are
 * implementation-defined.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test the conditions under which one process has permission to\nchange the scheduling parameters of another process, because they are\nimplementation-defined.\n");
        return PTS_UNTESTED;
}
