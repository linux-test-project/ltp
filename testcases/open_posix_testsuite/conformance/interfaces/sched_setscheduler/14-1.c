/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test that the underlying kernel-scheduled entities for the system
 * contention scope threads are not be affected by this function.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
        printf("Will not test that the underlying kernel-scheduled entities for the system\ncontention scope threads are not be affected by sched_setscheduler().\n");
        return PTS_UNTESTED;
}
