/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test the condition under which one process has the appropriate
 * privilege to change the scheduling parameters of another process because 
 * they are implementation-defined.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test the condition under which one process has the appropriate\nprivilege to change the scheduling parameters of another process because \nthey are implementation-defined.\n");
        return PTS_UNTESTED;
}
