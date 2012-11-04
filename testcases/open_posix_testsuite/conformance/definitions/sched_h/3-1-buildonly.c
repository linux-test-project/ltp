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
 * Test that when _POSIX_SPORADIC_SERVER is defined, sched_param structure
 * includes the following:
 *   int sched_ss_low_priority
 *   struct timespec sched_ss_repl_period
 *   struct timespec sched_ss_init_budget
 *   int sched_ss_max_repl
*/
#include <sched.h>
#include <unistd.h>

#if defined(_POSIX_SPORADIC_SERVER) && _POSIX_SPORADIC_SERVER != -1

struct sched_param s;

int dummyfcn(void)
{
	struct timespec ss_repl_period, ss_init_budget;

	s.sched_ss_low_priority = 0;
	ss_repl_period = s.sched_ss_repl_period;
	ss_init_budget = s.sched_ss_init_budget;
	s.sched_ss_max_repl = 0;

	return 0;
}

#endif
