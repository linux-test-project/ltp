/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test the behavior of sched_setparam() when pid is negative because
 * it is unspecified.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test the behavior of sched_setparam() when pid is negative because\nit is unspecified.\n");
        return PTS_UNTESTED;
}