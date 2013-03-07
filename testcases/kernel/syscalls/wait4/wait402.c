/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 *
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

 /*
  * wait402 - check for ECHILD errno when using an illegal pid value
  */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#define _USE_BSD
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

char *TCID = "wait402";
int TST_TOTAL = 1;

static void cleanup(void);
static void setup(void);

static int exp_enos[] = { ECHILD, 0 };

static long get_pid_max(void)
{
	long pid_max;

	SAFE_FILE_SCANF(NULL, "/proc/sys/kernel/pid_max", "%ld", &pid_max);

	return pid_max;
}

int main(int ac, char **av)
{
	int lc;
	char *msg;
	pid_t epid = get_pid_max() + 1;

	int status = 1;
	struct rusage rusage;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(wait4(epid, &status, 0, &rusage));

		if (TEST_RETURN == 0) {
			tst_brkm(TFAIL, cleanup,
				 "call failed to produce expected error - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		}

		switch (TEST_ERRNO) {
		case ECHILD:
			tst_resm(TPASS,
				 "received expected failure - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_brkm(TFAIL, cleanup,
				 "call failed to produce expected "
				 "error - errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;
}
