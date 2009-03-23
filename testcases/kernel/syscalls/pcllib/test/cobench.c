/*
 *  CoBench by Davide Libenzi ( Portable Coroutine Library bench tester )
 *  Copyright (C) 2003  Davide Libenzi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Davide Libenzi <davidel@xmailserver.org>
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pcl.h>

#define MIN_MEASURE_TIME 2000000ULL
#define CO_STACK_SIZE (8 * 1024)

static volatile unsigned long sw_counter;

static unsigned long long getustime(void)
{
	struct timeval tm;

	gettimeofday(&tm, NULL);
	return (unsigned long long)tm.tv_sec * 1000000ULL +
	    (unsigned long long)tm.tv_usec;
}

static void switch_bench(void *data)
{

	for (;;) {
		sw_counter--;
		co_resume();
	}
}

int main(int argc, char *argv[])
{
	int i, ntimes;
	coroutine_t coro;
	unsigned long nswitches;
	unsigned long long ts, te;

	fprintf(stdout, "measuring co_create+co_delete performance ... ");
	fflush(stdout);

	ntimes = 10000;
	do {
		ts = getustime();
		for (i = 0; i < ntimes; i++) {
			if ((coro = co_create(switch_bench, NULL, NULL,
					      CO_STACK_SIZE)) != NULL)
				co_delete(coro);
		}
		te = getustime();
		ntimes *= 4;
	} while ((te - ts) < MIN_MEASURE_TIME);

	fprintf(stdout, "%g usec\n", (double)(te - ts) / (double)ntimes);

	if ((coro = co_create(switch_bench, NULL, NULL, CO_STACK_SIZE)) != NULL) {
		fprintf(stdout, "measuring switch performance ... ");
		fflush(stdout);

		sw_counter = nswitches = 10000;
		do {
			ts = getustime();
			while (sw_counter)
				co_call(coro);
			te = getustime();
			sw_counter = (nswitches *= 4);
		} while ((te - ts) < MIN_MEASURE_TIME);

		fprintf(stdout, "%g usec\n",
			(double)(te - ts) / (double)(2 * nswitches));

		co_delete(coro);
	}

	return 0;
}
