/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Will not test the result of sched_setparam when the scheduling policy of the
 * target process is not SCHED_FIFO, SCHED_RR, or SCHED_SPORADIC. It is
 * implementation-defined.
 *
 * @pt:SS
 */

#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

#ifdef _POSIX_SPORADIC_SERVER
int main() {
	printf(" Will not test the result of sched_setparam when the scheduling policy of the\ntarget process is not SCHED_FIFO, SCHED_RR, or SCHED_SPORADIC.\nIt is implementation-defined.\n");
	return PTS_UNTESTED;
}

#else
int main() {
	printf("Does not support SS (SPORADIC SERVER)\n");
	return PTS_UNSUPPORTED;
}

#endif