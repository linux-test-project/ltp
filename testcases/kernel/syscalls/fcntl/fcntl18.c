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
 *	fcntl18.c
 *
 * DESCRIPTION
 *	Test to check the error conditions in fcntl system call
 *
 * USAGE
 *	fcntl18
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */

#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"

#define INVAL_FLAG	-1
#define INVAL_MIN	(-2147483647L-1L)

int fd;
char string[40] = "";

char *TCID = "fcntl18";
int TST_TOTAL = 1;
struct passwd *pass;

void setup(void);
void cleanup(void);
int fail;

int main(int ac, char **av)
{
	int retval;
	struct flock fl;
	int pid, status;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

/* //block1: */
#ifndef UCLINUX
	/* Skip since uClinux does not implement memory protection */
	tst_resm(TINFO, "Enter block 1");
	fail = 0;
	if ((fd = open("temp.dat", O_CREAT | O_RDWR, 0777)) < 0) {	//mode must be specified when O_CREATE is in the flag
		tst_resm(TFAIL, "file opening error");
		fail = 1;
	}

	/* Error condition if address is bad */
	retval = fcntl(fd, F_GETLK, (struct flock *)INVAL_FLAG);
	if (errno == EFAULT) {
		tst_resm(TPASS, "Test F_GETLK: for errno EFAULT PASSED");
	} else {
		tst_resm(TFAIL, "Test F_GETLK: for errno EFAULT FAILED");
		fail = 1;
	}
	if (fail) {
		tst_resm(TINFO, "Block 1 FAILED");
	} else {
		tst_resm(TINFO, "Block 1 PASSED");
	}
	tst_resm(TINFO, "Exit block 1");
#else
	tst_resm(TINFO, "Skip block 1 on uClinux");
#endif

/* //block2: */
#ifndef UCLINUX
	/* Skip since uClinux does not implement memory protection */
	tst_resm(TINFO, "Enter block 2");
	fail = 0;
	/* Error condition if address is bad */
	retval = fcntl(fd, F_GETLK, (struct flock *)INVAL_FLAG);
	if (errno == EFAULT) {
		tst_resm(TPASS, "Test F_GETLK: for errno EFAULT PASSED");
	} else {
		tst_resm(TFAIL, "Test F_GETLK: for errno EFAULT FAILED");
		fail = 1;
	}
	if (fail) {
		tst_resm(TINFO, "Block 2 FAILED");
	} else {
		tst_resm(TINFO, "Block 2 PASSED");
	}
	tst_resm(TINFO, "Exit block 2");
#else
	tst_resm(TINFO, "Skip block 2 on uClinux");
#endif

/* //block3: */
	tst_resm(TINFO, "Enter block 3");
	fail = 0;
	if ((pid = tst_fork()) == 0) {	/* child */
		fail = 0;
		pass = getpwnam("nobody");
		retval = setreuid(-1, pass->pw_uid);
		if (retval < 0) {
			tst_resm(TFAIL, "setreuid to user nobody failed, "
				 "errno: %d", errno);
			fail = 1;
		}

		/* Error condition: invalid cmd */
		retval = fcntl(fd, INVAL_FLAG, &fl);
		if (errno == EINVAL) {
			tst_resm(TPASS, "Test for errno EINVAL PASSED");
		} else {
			tst_resm(TFAIL, "Test for errno EINVAL FAILED, "
				 "got: %d", errno);
			fail = 1;
		}
		exit(fail);
	} else {		/* parent */
		waitpid(pid, &status, 0);
		if (WEXITSTATUS(status) != 0) {
			tst_resm(TFAIL, "child returned bad exit status");
			fail = 1;
		}
		if (fail) {
			tst_resm(TINFO, "Block 3 FAILED");
		} else {
			tst_resm(TINFO, "Block 3 PASSED");
		}
	}
	tst_resm(TINFO, "Exit block 3");

	cleanup();
	tst_exit();

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	umask(0);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(string, "./fcntl18.%d.1", getpid());
	unlink(string);
}

/*
 * cleanup()
 *	performs all the ONE TIME cleanup for this test at completion or
 *	or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified
	 */
	close(fd);

	tst_rmdir();

	unlink("temp.dat");

}
