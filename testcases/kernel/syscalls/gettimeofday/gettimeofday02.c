/*
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	gettimeofday02.c
 *
 * DESCRIPTION
 *	Check if gettimeofday is monotonous
 *
 * ALGORITHM
 *	Call gettimeofday() to get a t1 (fist value)
 *	call it again to get t2, see if t2 < t1, set t2 = t1, repeat for 30 sec
 *
 * USAGE:  <for command-line>
 *  gettimeofday02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *             -T   : Seconds to test gettimeofday (default 30)
 *
 * HISTORY
 *	05/2002 Written by Andi Kleen
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <test.h>
#include <usctest.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define gettimeofday(a,b)  syscall(__NR_gettimeofday,a,b)

char *TCID = "gettimeofday02";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int Tflag;
char *tlen = "30";

sig_atomic_t done;

option_t opts[] = { {"T:", &Tflag, &tlen}, {} };

void breakout(int sig)
{
	done = 1;
}

void cleanup(void)
{
	TEST_CLEANUP;

	tst_exit();
}

void help()
{
	printf("  -T len  seconds to test gettimeofday (default %s)\n", tlen);
}

int main(int ac, char **av)
{
	struct timeval tv1, tv2;
	char *msg;

	if ((msg = parse_opts(ac, av, opts, help)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	tst_resm(TINFO, "checking if gettimeofday is monotonous, takes %ss",
		 tlen);
	signal(SIGALRM, breakout);
	alarm(atoi(tlen));

	if (gettimeofday(&tv1, NULL) != 0)
		tst_brkm(TBROK, cleanup, "first gettimeofday() failed: %s\n",
			 strerror(errno));
	while (!done) {
		if (gettimeofday(&tv2, NULL) != 0)
			tst_brkm(TBROK, cleanup,
				 "loop gettimeofday() failed: %s\n",
				 strerror(errno));

		if (tv2.tv_sec < tv1.tv_sec ||
		    (tv2.tv_sec == tv1.tv_sec && tv2.tv_usec < tv1.tv_usec)) {
			tst_resm(TFAIL,
				 "Time is going backwards: old %jd.%jd vs new %jd.%jd!",
				 (intmax_t)tv1.tv_sec, (intmax_t)tv1.tv_usec, (intmax_t)tv2.tv_sec,
				 (intmax_t)tv2.tv_usec);
			cleanup();
			return 1;
		}

		tv1 = tv2;
	}

	tst_resm(TPASS, "gettimeofday monotonous in %s seconds", tlen);

	cleanup();
	return 0;
}
