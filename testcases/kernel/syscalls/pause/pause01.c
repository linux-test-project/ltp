/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: William Roske
 *    CO-PILOT		: Dave Fenner
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
 */
/*
 * Setup alarm() signal, wait in pause() expect to be woken up.
 */
#include <errno.h>
#include <signal.h>
#include "test.h"

char *TCID = "pause01";
int TST_TOTAL = 1;

static void setup(void);

int main(int ac, char **av)
{
	int lc;
	struct itimerval it = {
		.it_interval = {.tv_sec = 0, .tv_usec = 0},
		.it_value = {.tv_sec = 0, .tv_usec = 1000},
	};

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if (setitimer(ITIMER_REAL, &it, NULL))
			tst_brkm(TBROK | TERRNO, NULL, "setitimer() failed");

		TEST(pause());

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL,
				 "pause() returned WITHOUT an error return code : %d",
				 TEST_ERRNO);
		} else {
			if (TEST_ERRNO == EINTR)
				tst_resm(TPASS, "pause() returned %ld",
					 TEST_RETURN);
			else
				tst_resm(TFAIL,
					 "pause() returned %ld. Expected %d (EINTR)",
					 TEST_RETURN, EINTR);
		}
	}

	tst_exit();
}

static void go(int sig)
{
	(void)sig;
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, NULL);
	(void)signal(SIGALRM, go);

	TEST_PAUSE;
}
