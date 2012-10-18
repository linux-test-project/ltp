/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * @pt:SS
 * @pt:TSP
 * Test that SCHED_SPORADIC is defined
 */

#include <sched.h>
#include <unistd.h>

#if (defined(_POSIX_SPORADIC_SERVER) && _POSIX_SPORADIC_SERVER != -1) \
    || (defined(_POSIX_THREAD_SPORADIC_SERVER) && \
	_POSIX_THREAD_SPORADIC_SERVER != -1)

#ifndef SCHED_SPORADIC
#error SCHED_SPORADIC not defined
#endif

#endif
