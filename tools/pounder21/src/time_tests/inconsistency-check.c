/* Time inconsistency check test
 *		by: john stultz (johnstul@us.ibm.com)
 *		(C) Copyright IBM 2003, 2004, 2005
 */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define CALLS_PER_LOOP 64
#define NSEC_PER_SEC 1000000000ULL

/* returns 1 if a <= b, 0 otherwise */
static inline int in_order(struct timespec a, struct timespec b)
{
	if (a.tv_sec < b.tv_sec)
		return 1;
	if (a.tv_sec > b.tv_sec)
		return 0;
	if (a.tv_nsec > b.tv_nsec)
		return 0;
	return 1;
}

int main(int argc, char *argv[])
{
	struct timespec list[CALLS_PER_LOOP];
	int i, inconsistent;
	unsigned long seconds = -1;
	long now, then;
	int clock_type = CLOCK_MONOTONIC;

	if (argc > 1)
		seconds = atol(argv[1]);

	/* make sure CLOCK_MONOTONIC is supported */
	if (clock_gettime(clock_type, &list[0])) {
		printf("Using CLOCK_REALTIME\n");
		clock_type = CLOCK_REALTIME;
	}

	clock_gettime(clock_type, &list[0]);
	now = then = list[0].tv_sec;

	/* timestamp start of test */
	system("date");
	while (seconds == -1 || now - then < seconds) {
		inconsistent = 0;

		/* Fill list */
		for (i = 0; i < CALLS_PER_LOOP; i++)
			clock_gettime(clock_type, &list[i]);

		/* Check for inconsistencies */
		for (i = 0; i < CALLS_PER_LOOP - 1; i++)
			if (!in_order(list[i], list[i + 1]))
				inconsistent = i;

		/* display inconsistency */
		if (inconsistent) {
			unsigned long long delta;
			for (i = 0; i < CALLS_PER_LOOP; i++) {
				if (i == inconsistent)
					printf("--------------------\n");
				printf("%lu:%lu\n", list[i].tv_sec,
				       list[i].tv_nsec);
				if (i == inconsistent + 1)
					printf("--------------------\n");
			}
			delta = list[inconsistent].tv_sec * NSEC_PER_SEC;
			delta += list[inconsistent].tv_nsec;
			delta -= list[inconsistent + 1].tv_sec * NSEC_PER_SEC;
			delta -= list[inconsistent + 1].tv_nsec;
			printf("Delta: %llu ns\n", delta);
			fflush(0);
			/* timestamp inconsistency */
			system("date");
			return -1;
		}
		now = list[0].tv_sec;
	}
	return 0;
}
