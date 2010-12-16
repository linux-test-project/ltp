/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the sched_ss_repl_period and sched_ss_init_budget members of the
 * param argument represent the time parameters to be used by the sporadic
 * server scheduling policy for the target process.
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