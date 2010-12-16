/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the sched_ss_low_priority, sched_ss_repl_period, and
 * sched_ss_init_budget members of the param argument have no effect on the
 * scheduling behavior if the scheduling policy of the target process is either
 * SCHED_FIFO or SCHED_RR.

 *
 * @pt:SS
 */

#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#ifdef _POSIX_SPORADIC_SERVER
int main() {
	printf("Not yet tested.\n");
	return PTS_UNTESTED;
}

#else
int main() {
	printf("Does not support SS (SPORADIC SERVER)\n");
	return PTS_UNSUPPORTED;
}

#endif