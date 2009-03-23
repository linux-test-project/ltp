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
 *  Blocks 1-4 are with char[], #4 is special case where address is -1
 *
 *	Block 1: Call getcwd(2) with valid cwd[]:
 *              Should work fine
 *	Block 2: Call getcwd(2) with valid cwd[], size = 0:
 *              Should return NULL, errno = EINVAL
 *	Block 3: Call getcwd(2) with valid cwd[], size <= strlen(path):
 *              i.e. size = 1, Should return NULL, errno = ERANGE
 *      Block 4: Call getcwd(2) with cwd address = -1, size > strlen(path):
 *              Should return NULL, errno = EFAULT
 *
 *  Blocks 5-7 are with char*
 *
 *	Block 5: Call getcwd(2) with *buffer = NULL, size = 0:
 * 		Should allocate buffer, and work fine
 *	Block 6: Call getcwd(2) with *buffer = NULL, size <= strlen(path):
 * 		i.e. size = 1, Should return NULL, errno = ERANGE
 *      Block 7: Call getcwd(2) with *buffer = NULL, size > strlen(path):
 *              Should work fine and allocate buffer
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *      02/2002 Added more testcases, cleaned up by wjh
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
int TST_TOTAL = 7;
extern int Tst_count;

void cleanup(void);
void setup(void);
void do_block1();
void do_block2();
void do_block3();
void do_block4();
void do_block5();
void do_block6();
void do_block7();

char pwd_buf[BUFSIZ];		//holds results of pwd pipe
char cwd[BUFSIZ];		//used as our valid buffer
char *buffer = NULL;		//catches the return value from getcwd when passing NULL
char *cwd_ptr = NULL;		//catches the return value from getcwd() when passing cwd[]

int main(int ac, char **av)
{
	FILE *fin;
	char *cp, *cp_cur;
	int lc;			/* loop counter */
	char *msg;		/* parse_opts() return message */

	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}
	setup();

	/*
	 * The following loop checks looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		if ((fin = popen(pwd, "r")) == NULL) {
			tst_resm(TINFO, "%s: can't run %s", TCID, pwd);
			tst_brkm(TBROK, cleanup, "%s FAILED", TCID);
		 /*NOTREACHED*/}
		while (fgets(pwd_buf, sizeof(pwd_buf), fin) != NULL) {
			if ((cp = strchr(pwd_buf, '\n')) == (char *)NULL) {
				tst_brkm(TBROK, cleanup, "pwd output too long");
			 /*NOTREACHED*/}
			*cp = 0;
			cp_cur = pwd_buf;
		}
		pclose(fin);

		do_block1();
		do_block2();
		do_block3();
		do_block4();
		do_block5();
		do_block6();
		do_block7();
	}
	cleanup();
	 /*NOTREACHED*/ return 0;
}

void do_block1()		//valid cwd[]: -> Should work fine
{
	int flag = 0;
	tst_resm(TINFO, "Enter Block 1");

	if ((cwd_ptr = getcwd(cwd, sizeof(cwd))) == NULL) {
		tst_resm(TFAIL, "getcwd() failed unexpectedly: "
			 "errno = %d\n", errno);
		flag = FAILED;
	}
	if ((flag != FAILED) && (strcmp(pwd_buf, cwd) != 0)) {
		tst_resm(TFAIL, "getcwd() returned unexpected working "
			 "directory: expected: %s, got: %s\n", pwd_buf, cwd);
		flag = FAILED;
	}
	tst_resm(TINFO, "Exit Block 1");
	if (flag == FAILED) {
		tst_resm(TFAIL, "Block 1 FAILED");
	} else {
		tst_resm(TPASS, "Block 1 PASSED");
	}
}

void do_block2()		//valid cwd[], size = 0: -> Should return NULL, errno = EINVAL
{
	int flag = 0;
	tst_resm(TINFO, "Enter Block 2");

	if (((cwd_ptr = getcwd(cwd, 0)) == NULL)
	    && (errno != EINVAL)) {
		tst_resm(TFAIL, "getcwd() failed unexpectedly: "
			 "errno = %d expected EINVAL(%d)\n", errno, EINVAL);
		flag = FAILED;
	}
	tst_resm(TINFO, "Exit Block 2");
	if (flag == FAILED) {
		tst_resm(TFAIL, "Block 2 FAILED");
	} else {
		tst_resm(TPASS, "Block 2 PASSED");
	}
}

void do_block3()		//valid cwd[], size = 1 -> Should return NULL, errno = ERANGE
{
	int flag = 0;
	tst_resm(TINFO, "Enter Block 3");

	if (((cwd_ptr = getcwd(cwd, 1)) != NULL)
	    || (errno != ERANGE)) {
		tst_resm(TFAIL, "getcwd() failed unexpectedly: "
			 "errno = %d, expected ERANGE(%d)\n", errno, ERANGE);
		flag = FAILED;
	}
	tst_resm(TINFO, "Exit Block 3");
	if (flag == FAILED) {
		tst_resm(TFAIL, "Block 3 FAILED");
	} else {
		tst_resm(TPASS, "Block 3 PASSED");
	}
}

void do_block4()		//invalid cwd[] = -1, size = BUFSIZ: -> return NULL, errno = FAULT
{
/* Skip since uClinux does not implement memory protection */
#ifndef UCLINUX
	int flag = 0;
	tst_resm(TINFO, "Enter Block 4");

	if (((cwd_ptr = getcwd((char *)-1, sizeof(cwd))) != NULL)
	    || (errno != EFAULT)) {
		tst_resm(TFAIL, "getcwd() failed unexpectedly: "
			 "errno = %d, expected EFAULT(%d)\n", errno, EFAULT);
		flag = FAILED;
	}
	tst_resm(TINFO, "Exit Block 4");
	if (flag == FAILED) {
		tst_resm(TFAIL, "Block 4 FAILED");
	} else {
		tst_resm(TPASS, "Block 4 PASSED");
	}
#else
	tst_resm(TINFO, "Skipping Block 4 on uClinux");
#endif
}

void do_block5()		//buffer = NULL, and size = 0, should succeed
{
	int flag = 0;
	tst_resm(TINFO, "Enter Block 5");

	if ((buffer = getcwd(NULL, 0)) == NULL) {
		tst_resm(TFAIL, "getcwd() failed unexpectedly: "
			 "errno = %d\n", errno);
		flag = FAILED;
	}
	if ((flag != FAILED) && (strcmp(pwd_buf, buffer) != 0)) {
		tst_resm(TFAIL, "getcwd() returned unexpected working "
			 "directory: expected: %s, got: %s\n", pwd_buf, buffer);
		flag = FAILED;
	}
	tst_resm(TINFO, "Exit Block 5");
	if (flag == FAILED) {
		tst_resm(TFAIL, "Block 5 FAILED");
	} else {
		tst_resm(TPASS, "Block 5 PASSED");
	}
	free(buffer);
	buffer = NULL;
}

void do_block6()		//buffer = NULL, size = 1: -> return NULL, errno = ERANGE
{
	int flag = 0;
	tst_resm(TINFO, "Enter Block 6");

	if (((buffer = getcwd(NULL, 1)) != NULL)
	    || (errno != ERANGE)) {
		tst_resm(TFAIL, "getcwd() failed unexpectedly: "
			 "errno = %d, expected ERANGE(%d)\n", errno, ERANGE);
		flag = FAILED;
	}
	tst_resm(TINFO, "Exit Block 6");
	if (flag == FAILED) {
		tst_resm(TFAIL, "Block 6 FAILED");
	} else {
		tst_resm(TPASS, "Block 6 PASSED");
	}
}

void do_block7()		//buffer = NULL, size = BUFSIZ: -> work fine, allocate buffer
{
	int flag = 0;
	tst_resm(TINFO, "Enter Block 7");

	if ((buffer = getcwd(NULL, sizeof(cwd))) == NULL) {
		tst_resm(TFAIL, "getcwd() failed unexpectedly: "
			 "errno = %d\n", errno);
		flag = FAILED;
	}
	if ((flag != FAILED) && (strcmp(pwd_buf, buffer) != 0)) {
		tst_resm(TFAIL, "getcwd() returned unexpected working "
			 "directory: expected: %s, got: %s\n", pwd_buf, buffer);
		flag = FAILED;
	}
	tst_resm(TINFO, "Exit Block 7");
	if (flag == FAILED) {
		tst_resm(TFAIL, "Block 7 FAILED");
	} else {
		tst_resm(TPASS, "Block 7 PASSED");
	}
	free(buffer);
	buffer = NULL;
}

void setup()
{
	/* capture signals */
	/* FORK is set here because of the popen() call above */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* create a test directory and cd into it */
	tst_tmpdir();
}

void cleanup()
{
	/* remove the test directory */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
