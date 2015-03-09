/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 * NAME
 *	uname03.c
 *
 * DESCRIPTION
 *	uname03 - call uname() and make sure it succeeds
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	check the errno value
 *	  issue a PASS message if we get zero
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  uname03 [-c n] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	none
 */

#include "test.h"

#include <errno.h>
#include <sys/utsname.h>
#include <string.h>

void cleanup(void);
void setup(void);

char *TCID = "uname03";
int TST_TOTAL = 1;

#define LINUX	"Linux"

int main(int ac, char **av)
{
	int lc;
	struct utsname *buf;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* allocate some space for buf */

	if ((buf = malloc((size_t)sizeof(struct utsname))) == NULL) {
		tst_brkm(TBROK, cleanup, "malloc failed for buf");
	}

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/* Now make the system call with the TEST() macro */

		TEST(uname(buf));

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "%s failed - errno = %d - %s",
				 TCID, TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if ((strcmp(buf->sysname, LINUX)) == 0) {
				tst_resm(TPASS, "%s functionality test "
					 "succeeded", TCID);
			} else {
				tst_resm(TFAIL, "%s functionality test "
					 "failed", TCID);
			}
		}
	}

	free(buf);
	buf = NULL;

	cleanup();
	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{

}
