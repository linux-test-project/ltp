/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/*
 *    AUTHOR            : Richard Logan
 *    CO-PILOT          : Glen Overby
 *    DATE STARTED      : 02/24/93
 *
 *    TEST CASES
 *      1.) select(2) to fd of system pipe with no I/O and small timeout
 */
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>

#include "test.h"
#include "safe_macros.h"

static void setup(void);

char *TCID = "select02";
int TST_TOTAL = 1;

int Fd[2];
fd_set saved_Readfds, saved_Writefds;
fd_set Readfds, Writefds;

int main(int ac, char **av)
{
	int lc;
	struct timeval timeout;
	long test_time = 0;	/* in usecs */

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		test_time = ((lc % 2000) * 100000);	/* 100 milli-seconds */

		if (test_time > 1000000 * 60)
			test_time = test_time % (1000000 * 60);

		timeout.tv_sec = test_time / 1000000;
		timeout.tv_usec = test_time - (timeout.tv_sec * 1000000);

		Readfds = saved_Readfds;
		Writefds = saved_Writefds;

		TEST(select(5, &Readfds, &Writefds, 0, &timeout));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL,
				 "%d select(5, &Readfds, &Writefds, 0, &timeout) failed, errno=%d\n",
				 lc, errno);
		} else {
			tst_resm(TPASS,
				 "select(5, &Readfds, &Writefds, 0, &timeout) timeout = %ld usecs",
				 test_time);
		}

	}

	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, NULL);

	TEST_PAUSE;

	SAFE_PIPE(NULL, Fd);

	FD_ZERO(&saved_Readfds);
	FD_ZERO(&saved_Writefds);
	FD_SET(Fd[0], &saved_Readfds);
	FD_SET(Fd[1], &saved_Writefds);
}
