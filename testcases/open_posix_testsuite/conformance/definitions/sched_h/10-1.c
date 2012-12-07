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
 * The policies symbols shall have unique value
 * I try to test every symbol in a loop.
 */
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include "posixtest.h"

struct unique {
	int value;
	char *name;
} sym[] = {

	{
	SCHED_FIFO, "SCHED_FIFO"}, {
	SCHED_RR, "SCHED_RR"},
#if defined(_POSIX_SPORADIC_SERVER) && _POSIX_THREAD_SPORADIC_SERVER == 200112L
	{
	SCHED_SPORADIC, "SCHED_SPORADIC"},
#endif
	{
	SCHED_OTHER, "SCHED_OTHER"}, {
	0, 0}
};

int main()
{
	struct unique *tst;
	int i, ret = PTS_PASS;
	tst = sym;

	while (tst->name) {
		for (i = 0; sym[i].name; i++) {
			if (tst->value == sym[i].value
			    && strcmp(tst->name, sym[i].name)) {
				printf("%s has a duplicate value with %s\n",
				       tst->name, sym[i].name);
				ret = PTS_FAIL;
			}
		}
		tst++;
	}
	return ret;
}
