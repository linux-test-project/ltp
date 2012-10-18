/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the sched_ss_max_repl member of the param argument represent the
 * maximum number of replenishments that are allowed to be pending
 * simultaneously for the process scheduled under the SCHED_SPORADIC policy.
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
