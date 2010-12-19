/*
 * Copyright (c) Wipro Technologies Ltd, 2003.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**************************************************************************
 *
 *    TEST IDENTIFIER	: ustat01
 *
 *    EXECUTED BY	: Anyone
 *
 *    TEST TITLE	: Basic test for ustat(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *    This is a Phase I test for the ustat(2) system call.
 *    It is intended to provide a limited exposure of the system call.
 *
 * 	Setup:
 *	 Setup signal handling.
 *	 Pause for SIGUSR1 if option specified.
 *	 Find out device number for a particular file.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 * ustat01 [-c n] [-e] [-i n] [-I x] [-p x] [-t] [-h] [-f] [-p]
 * where:
 * 	-c n : run the test for n number of times.
 *	-e   : Turn on errno logging.
 *	-i n : Execute test n times.
 *	-I x : Execute test for x seconds.
 *	-p   : Pause for SIGUSR1 before starting
 *	-P x : Pause for x seconds between iterations.
 *	-t   : Turn on syscall timing.
 *      -f   : Turn of functionality testing
 *
 *RESTRICTIONS: None
 *****************************************************************************/
#include "test.h"
#include "usctest.h"
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>		/* libc[45] */
#include <ustat.h>		/* glibc2 */
#include <sys/stat.h>

static void setup();
static void cleanup();

char *TCID = "ustat01";
int TST_TOTAL = 1;

dev_t dev_num;
struct ustat *ubuf;
struct stat *buf;

int main(int argc, char *argv[])
{
	int lc, i;
	char *msg;

	/*parse standard options */
	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(ustat(dev_num, ubuf));
			/* check return code */
			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL, "ustat(2) failed and set"
					 "the errno to %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			} else {
				tst_resm(TPASS, "ustat(2) passed");
			}
		}
	}
	cleanup();
	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Allocate memory for stat and ustat structure variables */
	if ((buf = (struct stat *)malloc(sizeof(struct stat))) == NULL) {
		tst_brkm(TBROK, NULL, "Failed to allocate Memory");
	}

	if ((ubuf = (struct ustat *)malloc(sizeof(struct ustat))) == NULL) {
		free(buf);
		tst_brkm(TBROK, NULL, "Failed to allocate Memory");
	}

	/*Find out device number for a file-system */
	if (stat("/", buf) != 0) {
		free(buf);
		free(ubuf);
		tst_brkm(TBROK, NULL, "Couldn't find device number");
	}

	dev_num = buf->st_dev;
}

/*
 * cleanup() - Performs one time cleanup for this test at
 * completion or premature exit
 */
void cleanup()
{

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}