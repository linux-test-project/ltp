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
extern int  Tst_count;
extern char *TESTDIR;                /* temporary dir created by tst_tmpdir() */
/* Global Variables */
char *TCID     = "performance_counter01"; /* test program identifier.          */
int  TST_TOTAL = 1;

enum hw_event_types {
	PERF_COUNT_CYCLES,
	PERF_COUNT_INSTRUCTIONS,
	PERF_COUNT_CACHE_REFERENCES,
	PERF_COUNT_CACHE_MISSES,
	PERF_COUNT_BRANCH_INSTRUCTIONS,
	PERF_COUNT_BRANCH_MISSES,
};

void cleanup(void) { /* Stub function. */ }

int
main(void) {

	unsigned long long count1, count2;
	int fd1, fd2, ret;

	fd1 = syscall(__NR_perf_event_open,
			PERF_COUNT_INSTRUCTIONS, 0, 0, 0, -1);
	if (fd1 < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			"Failed to create PERF_COUNT_INSTRUCTIONS fd");
	}
	fd2 = syscall(__NR_perf_event_open,
			PERF_COUNT_CACHE_MISSES, 0, 0, 0, -1);
	if (fd2 < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			"Failed to create PERF_COUNT_CACHE_MISSES fd");
	}

	do {

		ret = read(fd1, &count1, sizeof(count1));

		if (ret == sizeof(count1)) {

			ret = read(fd2, &count2, sizeof(count2));

			if (ret == sizeof(count2)) {
				tst_resm(TINFO,
					"counter1 value: %Ld instructions",
					count1);
				tst_resm(TINFO,
					"counter2 value: %Ld cachemisses",
					count2);
				sleep(1);
			}

		}

	} while (ret == sizeof(unsigned long long));

	tst_exit();

}
