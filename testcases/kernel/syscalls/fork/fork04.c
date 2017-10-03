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
 *
 *    OS Test - Silicon Graphics, Inc.
 *    TEST IDENTIFIER	: fork04
 *    TEST TITLE	: Child inheritance of Environment Variables after fork()
 *    PARENT DOCUMENT	: frktds01
 *    TEST CASE TOTAL	: 3
 *    WALL CLOCK TIME	: 1
 *    CPU TYPES		: ALL
 *    AUTHOR		: Kathy Olmsted
 *    CO-PILOT		: Steve Shaw
 *    DATE STARTED	: 06/17/92
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *       Test these environment variables correctly inherited by child:
 *       1. TERM
 *       2. NoTSetzWq
 *       3. TESTPROG
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    DURATION
 * 	Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    DETAILED DESCRIPTION
 *
 * 	Setup:
 * 	  Setup signal handling.
 *        Make and change to a temporary directory.
 *	  Pause for SIGUSR1 if option specified.
 *        Add TESTPROG variable to the environment
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	 fork()
 *	 Check return code, if system call failed (return=-1)
 *		Log the errno
 *	   CHILD:
 *              open a temp file
 *		Determine environment values and write to file
 *		close file containing test values.
 *		exit.
 *	    PARENT:
 *		Wait for child to exit.
 *              Verify exit status
 *		Open file containing test values.
 *		For each test case:
 *			Read the value from the file.
 *			Determine and report PASS/FAIL result.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *        Remove the temporary directory and exit.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/param.h>
#include <signal.h>
#include <errno.h>
#include "test.h"
#include "safe_macros.h"

char *TCID = "fork04";

#define	KIDEXIT	42
#define MAX_LINE_LENGTH 256
#define OUTPUT_FILE  "env.out"
#define ENV_NOT_SET  "getenv() does not find variable set"

/* list of environment variables to test */
char *environ_list[] = { "TERM", "NoTSetzWq", "TESTPROG" };

#define NUMBER_OF_ENVIRON (sizeof(environ_list)/sizeof(char *))
int TST_TOTAL = NUMBER_OF_ENVIRON;

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
	tst_tmpdir();

	/* add a variable to the environment */
	putenv("TESTPROG=FRKTCS04");
}

static void child_environment(void)
{

	int fildes;
	int index;
	char msg[MAX_LINE_LENGTH];
	char *var;

	fildes = creat(OUTPUT_FILE, 0700);

	for (index = 0; index < NUMBER_OF_ENVIRON; index++) {
		memset(msg, 0, MAX_LINE_LENGTH);

		var = getenv(environ_list[index]);
		if (var == NULL)
			(void)sprintf(msg, "%s:%s", environ_list[index],
				      ENV_NOT_SET);
		else
			(void)sprintf(msg, "%s:%s", environ_list[index], var);
		/* includes extra null chars */
		write(fildes, msg, sizeof(msg));
	}

	close(fildes);
}

/*
 * Compare parent env string to child's string.
 * Each string is in the format:  <env var>:<value>
 */
static int cmp_env_strings(char *pstring, char *cstring)
{
	char *penv, *cenv, *pvalue, *cvalue;

	/*
	 * Break pstring into env and value
	 */
	penv = pstring;
	pvalue = strchr(pstring, ':');
	if (pvalue == NULL) {
		tst_resm(TBROK,
			 "internal error - parent's env string not in correct format:'%s'",
			 pstring);
		return -1;
	} else {
		*pvalue = '\0';
		pvalue++;
		if (*pvalue == '\0') {
			tst_resm(TBROK,
				 "internal error - missing parent's env value");
			return -1;
		}
	}

	/*
	 * Break cstring into env and value
	 */
	cenv = cstring;
	cvalue = strchr(cstring, ':');
	if (cvalue == NULL) {
		tst_resm(TBROK,
			 "internal error - parent's env string not in correct format:'%s'",
			 cstring);
		return -1;
	} else {
		*cvalue = '\0';
		cvalue++;
		if (*cvalue == '\0') {
			tst_resm(TBROK,
				 "internal error - missing child's env value");
			return -1;
		}
	}

	if (strcmp(penv, cenv) != 0) {
		tst_resm(TBROK, "internal error - parent(%s) != child (%s) env",
			 penv, cenv);
		return -1;
	}

	if (strcmp(pvalue, cvalue) != 0) {
		tst_resm(TFAIL,
			 "Env var %s changed after fork(), parent's %s, child's %s",
			 penv, pvalue, cvalue);
	} else {
		tst_resm(TPASS, "Env var %s unchanged after fork(): %s",
			 penv, cvalue);
	}
	return 0;

}

/***************************************************************
 * parent_environment - the parent side of the environment tests
 *        determine values for the variables
 *        read the values determined by the child
 *        compare values
 ***************************************************************/
void parent_environment(void)
{

	int fildes;
	char tmp_line[MAX_LINE_LENGTH];
	char parent_value[MAX_LINE_LENGTH];
	int index;
	int ret;
	char *var;

	fildes = SAFE_OPEN(cleanup, OUTPUT_FILE, O_RDWR);
	for (index = 0; index < NUMBER_OF_ENVIRON; index++) {
		ret = read(fildes, tmp_line, MAX_LINE_LENGTH);
		if (ret == 0) {
			tst_resm(TBROK,
				 "fork() test. parent_environment: failed to read from file with %d (%s)",
				 errno, strerror(errno));
		} else {

			var = getenv(environ_list[index]);
			if (var == NULL)
				sprintf(parent_value, "%s:%s",
					environ_list[index], ENV_NOT_SET);
			else
				sprintf(parent_value, "%s:%s",
					environ_list[index], var);

			cmp_env_strings(parent_value, tmp_line);

		}
	}

	close(fildes);
}

int main(int ac, char **av)
{
	int lc;
	int kid_status;
	int wait_status;
	int fails;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		fails = 0;

		TEST(fork());

		if (TEST_RETURN == -1) {
			/* fork failed */
			tst_brkm(TFAIL, cleanup,
				 "fork() failed with %d (%s)",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else if (TEST_RETURN == 0) {
			/* child */
			/* determine environment variables */
			child_environment();
			/* exit with known value */
			exit(KIDEXIT);
		} else {
			/* parent of successful fork */
			/* wait for the child to complete */
			wait_status = waitpid(TEST_RETURN, &kid_status, 0);
			/* validate the child exit status */
			if (wait_status == TEST_RETURN) {
				if (kid_status != KIDEXIT << 8) {
					tst_brkm(TBROK, cleanup,
						 "fork(): Incorrect child status returned on wait(): %d",
						 kid_status);
					fails++;
				}
			} else {
				tst_brkm(TBROK, cleanup,
					 "fork(): wait() for child status failed with %d errno: %d : %s",
					 wait_status, errno,
					 strerror(errno));
				fails++;
			}

			if (fails == 0) {
				/* verification tests */
				parent_environment();
			}
		}

	}

	cleanup();
	tst_exit();
}
