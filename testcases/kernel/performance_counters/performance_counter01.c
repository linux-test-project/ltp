/******************************************************************************/
/*                                                                            */
/* Ingo Molnar <mingo@elte.hu>, 2009					      */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/

/*
 * Very simple performance counter testcase.
 * Picked up from: http://lkml.org/lkml/2008/12/5/17
 */
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <linux/unistd.h>

#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

/* Extern Global Variables */
extern int  Tst_count;               /* counter for tst_xxx routines.         */
extern char *TESTDIR;                /* temporary dir created by tst_tmpdir() */
/* Global Variables */
char *TCID     = "performance_counter01"; /* test program identifier.          */
int  TST_TOTAL = 1; 

static void cleanup(void) { /* Stub function. */ }

int perf_counter_open(int		hw_event_type,
                      unsigned int	hw_event_period,
                      unsigned int	record_type,
                      pid_t		pid,
                      int		cpu)
{
	return syscall(__NR_perf_counter_open, hw_event_type, hw_event_period,
			record_type, pid, cpu);
}

enum hw_event_types {
	PERF_COUNT_CYCLES,
	PERF_COUNT_INSTRUCTIONS,
	PERF_COUNT_CACHE_REFERENCES,
	PERF_COUNT_CACHE_MISSES,
	PERF_COUNT_BRANCH_INSTRUCTIONS,
	PERF_COUNT_BRANCH_MISSES,
};

int
main(void) {
	unsigned long long count1, count2;
	int fd1, fd2, ret;
	fd1 = perf_counter_open(PERF_COUNT_INSTRUCTIONS, 0, 0, 0, -1);
	if (fd1 < 0) {
		tst_resm(TBROK | TERRNO,
			"Failed to create PERF_COUNT_INSTRUCTIONS fd");
		tst_exit();
	}
	fd2 = perf_counter_open(PERF_COUNT_CACHE_MISSES, 0, 0, 0, -1);
	if (fd2 < 0) {
		tst_resm(TBROK | TERRNO,
			"Failed to create PERF_COUNT_CACHE_MISSES fd");
		tst_exit();
	}

	for (;;) {
		ret = read(fd1, &count1, sizeof(count1));
		assert(ret == 8);
		ret = read(fd2, &count2, sizeof(count2));
		assert(ret == 8);
		printf("counter1 value: %Ld instructions\n", count1);
		printf("counter2 value: %Ld cachemisses\n",  count2);
		sleep(1);
	}
	return 0;
}

