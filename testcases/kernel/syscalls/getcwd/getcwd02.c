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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	getcwd02
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of the getcwd(2) system call.
 *
 * ALGORITHM
 *	Get the path name of the current working directory from the current
 *	shell through a pipe, and compare it with what is returned by
 *	getcwd(2) system call.
 *
 *	Block 1: Call getcwd(2) with a valid "buf"
 *	Block 2: Call getcwd(2) with buf = NULL
 *	Block 3: Call getcwd(2) with a valid buf, and size = 0
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>
#define FAILED 1

char *pwd = "/bin/pwd";
int flag;
char *TCID = "getcwd02";
int TST_TOTAL = 1;
extern int Tst_count;

void cleanup(void);
void setup(void);

main(int ac, char **av)
{
	FILE *fin;
	char buf[BUFSIZ];
	char cwd[BUFSIZ];
	char *cp, *cp_cur;
	char *buffer, *cwdptr;
	int lc;				/* loop counter */
	char *msg;			/* parse_opts() return message */

	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}
	setup();

	/*
	 * The following loop checks looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		if ((fin = popen(pwd, "r")) == NULL) {
			tst_resm(TINFO, "%s: can't run %s\n", TCID, pwd);
			tst_brkm(TBROK, cleanup, "%s FAILED", TCID);
			/*NOTREACHED*/
		}
		while (fgets(buf, sizeof(buf), fin) != NULL) {
			if ((cp = strchr(buf, '\n')) == (char *)NULL) {
				tst_brkm(TBROK, cleanup, "pwd output too long");
				/*NOTREACHED*/
			}
			*cp = 0;
			cp_cur = buf;
		}
		pclose(fin);

block1:
		tst_resm(TINFO, "Enter Block 1");
		flag = 0;

		if ((cwdptr = getcwd(cwd, sizeof cwd)) == NULL) {
			tst_resm(TFAIL, "getcwd() failed unexpectedly: "
				 "errno = %d\n", errno);
			flag = FAILED;
		}
		if ((flag != FAILED) && (strcmp(buf, cwd) != 0)) {
			tst_resm(TFAIL, "getcwd() returned unexpected working "
				 "directory: expected: %s, got: %s\n",
				 buf, cwd);
			flag = FAILED;
		}
		tst_resm(TINFO, "Exit Block 1");
		if (flag == FAILED) {
			tst_resm(TINFO, "Block 1 FAILED");
		} else {
			tst_resm(TPASS, "Block 1 PASSED");
		}

block2:
		tst_resm(TINFO, "Enter block 2");
		flag = 0;

		if ((cwdptr = getcwd(NULL, sizeof(buf))) == NULL) {
			tst_resm(TFAIL, "getcwd() failed unexpectedly: "
				 "errno = %d\n", errno);
			flag = FAILED;
		}
		if ((flag != FAILED) && strcmp(buf, cwdptr) != 0) {
			tst_resm(TFAIL, "getcwd() returned unexpected working "
				 "directory: expected: %s, got: %s\n",
				 buf, cwd);
			flag = FAILED;
		}
		tst_resm(TINFO, "Exit Block 2");
		if (flag == FAILED) {
			tst_resm(TINFO, "Block 2 FAILED");
		} else {
			tst_resm(TPASS, "Block 2 PASSED");
		}

block3:
		tst_resm(TINFO, "Enter block 3");
		flag = 0;

		if ((buffer = getcwd(buffer, 0)) == NULL) {
			tst_resm(TFAIL, "getcwd() failed unexpectedly: "
				 "errno = %d\n", errno);
			flag = FAILED;
		}
		if ((flag != FAILED) && strcmp(buffer, cwdptr) != 0) {
			tst_resm(TFAIL, "getcwd() returned unexpected working "
				 "directory: expected: %s, got: %s\n",
				 cwd, buffer);
			flag = FAILED;
		}
		tst_resm(TINFO, "Exit Block 3");
		if (flag == FAILED) {
			tst_resm(TINFO, "Block 3 FAILED");
		} else {
			tst_resm(TPASS, "Block 3 PASSED");
		}
	}
	cleanup();

	/*NOTREACHED*/
}

void
setup()
{
	/* capture signals */
	/* FORK is set here because of the popen() call above */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* create a test directory and cd into it */
	tst_tmpdir();
}

void
cleanup()
{
	/* remove the test directory */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
