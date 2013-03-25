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
 * Test that the values that can be returned by sched_getscheduler() are
 * defined in the sched.h header
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "posixtest.h"

struct unique {
	int value;
	char *name;
} sym[] = {
#ifdef SCHED_FIFO
	{
	SCHED_FIFO, "SCHED_FIFO"},
#endif
#ifdef SCHED_RR
	{
	SCHED_RR, "SCHED_RR"},
#endif
#ifdef SCHED_SPORADIC
	{
	SCHED_SPORADIC, "SCHED_SPORADIC"},
#endif
#ifdef SCHED_OTHER
	{
	SCHED_OTHER, "SCHED_OTHER"},
#endif
	{
	0, 0}
};

int main(void)
{
	int result = -1;
	struct unique *tst;

	tst = sym;
	result = sched_getscheduler(0);

	if (result == -1) {
		printf("Returned code is -1.\n");
		return PTS_FAIL;
	}
	if (errno != 0) {
		perror("Unexpected error");
		return PTS_FAIL;
	}

	while (tst->name) {
		if (result == tst->value) {
			printf("Test PASSED\n");
			return PTS_PASS;
		}
		tst++;
	}

	printf("The resulting scheduling policy is not one of standard "
	       "policy.\nIt could be an implementation defined policy.");
	return PTS_UNRESOLVED;
}
