/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name: nanosleep01
 *
 * Test Description:
 *  Verify that nanosleep() will be successful to suspend the execution
 *  of a process for a specified time.
 *
 * Expected Result:
 *  nanosleep() should return with value 0 and the process should be
 *  suspended for time specified by timespec structure.
 */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdint.h>

#include "test.h"

char *TCID = "nanosleep01";
int TST_TOTAL = 1;

static void setup(void);

int main(int ac, char **av)
{
	int lc;
	const char *msg;
	pid_t cpid;
	struct timeval otime;
	struct timeval ntime;
	int retval = 0, e_code, status;
	struct timespec timereq = {.tv_sec = 2, .tv_nsec = 9999};

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		/*
		 * Creat a child process and suspend it till
		 * time specified by timespec struct element
		 * time_t tv_sec.
		 */
		cpid = FORK_OR_VFORK();
		if (cpid == -1)
			tst_brkm(TBROK, NULL, "fork() failed");

		if (cpid == 0) {
			/* Note down the current time */
			gettimeofday(&otime, 0);
			/*
			 * Call nanosleep() to suspend child process
			 * for specified time.
			 */
			TEST(nanosleep(&timereq, NULL));

			/* time after child resumes execution */
			gettimeofday(&ntime, 0);

			if (TEST_RETURN == -1) {
				retval = 1;
				tst_resm(TFAIL,
					 "nanosleep() failed, errno=%d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
				continue;
			}

			/*
			 * Verify whether child execution was
			 * actually suspended to desired interval.
			 */
			long want_ms, got_ms;
			want_ms =
			    timereq.tv_sec * 1000 +
			    timereq.tv_nsec / 1000000;
			got_ms =
			    ntime.tv_sec * 1000 + ntime.tv_usec / 1000;
			got_ms -=
			    otime.tv_sec * 1000 + otime.tv_usec / 1000;
			if (got_ms < want_ms) {
				retval = 1;
				tst_resm(TFAIL, "Child execution not "
					 "suspended for %jd seconds.  (Wanted %ld ms, got %ld ms)",
					 (intmax_t) timereq.tv_sec,
					 want_ms, got_ms);
			} else {
				tst_resm(TPASS, "nanosleep "
					 "functionality is correct");
			}
			exit(retval);
		} else {
			/* wait for the child to finish */
			wait(&status);
			/* make sure the child returned a good exit status */
			e_code = status >> 8;
			if (e_code != 0) {
				tst_resm(TFAIL, "Failures reported above");
			}
		}
	}

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, NULL);

	TEST_PAUSE;
}
