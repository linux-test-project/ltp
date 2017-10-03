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
 *	ioctl01.c
 *
 * DESCRIPTION
 *	Testcase to check the errnos set by the ioctl(2) system call.
 *
 * ALGORITHM
 *	1. EBADF: Pass an invalid fd to ioctl(fd, ..) and expect EBADF.
 *	2. EFAULT: Pass an invalid address of arg in ioctl(fd, .., arg)
 *	3. EINVAL: Pass invalid cmd in ioctl(fd, cmd, arg)
 *	4. ENOTTY: Pass an non-streams fd in ioctl(fd, cmd, arg)
 *	5. EFAULT: Pass a NULL address for termio
 *
 * USAGE:  <for command-line>
 *  ioctl01 -D /dev/tty[0-9] [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	04/2002 Fixes by wjhuie
 *
 * RESTRICTIONS
 *      test must be run with the -D option
 *      test may have to be run as root depending on the tty permissions
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termio.h>
#include <termios.h>
#include "test.h"
#include "safe_macros.h"

char *TCID = "ioctl01";
int TST_TOTAL = 5;

#define	INVAL_IOCTL	9999999

static void setup(void);
static void cleanup(void);
static void help(void);

static int fd, fd1;
static int bfd = -1;

static struct termio termio;

static struct test_case_t {
	int *fd;
	int request;
	struct termio *s_tio;
	int error;
} TC[] = {
	/* file descriptor is invalid */
	{
	&bfd, TCGETA, &termio, EBADF},
	    /* termio address is invalid */
	{
	&fd, TCGETA, (struct termio *)-1, EFAULT},
	    /* command is invalid */
	    /* This errno value was changed from EINVAL to ENOTTY
	     * by kernel commit 07d106d0 and bbb63c51
	     */
	{
	&fd, INVAL_IOCTL, &termio, ENOTTY},
	    /* file descriptor is for a regular file */
	{
	&fd1, TCGETA, &termio, ENOTTY},
	    /* termio is NULL */
	{
	&fd, TCGETA, NULL, EFAULT}
};

static int Devflag;
static char *devname;

static option_t options[] = {
	{"D:", &Devflag, &devname},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, options, &help);

	if (!Devflag)
		tst_brkm(TBROK, NULL, "You must specify a tty device with "
			 "the -D option.");

	tst_require_root();

	setup();

	fd = SAFE_OPEN(cleanup, devname, O_RDWR, 0777);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(ioctl(*(TC[i].fd), TC[i].request, TC[i].s_tio));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == TC[i].error)
				tst_resm(TPASS | TTERRNO, "failed as expected");
			else
				tst_resm(TFAIL | TTERRNO,
					 "failed unexpectedly; expected %d - %s",
					 TC[i].error, strerror(TC[i].error));
		}
	}
	cleanup();

	tst_exit();
}

static void help(void)
{
	printf("  -D <tty device> : for example, /dev/tty[0-9]\n");
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	if (tst_kvercmp(3, 7, 0) < 0) {
		TC[2].error = EINVAL;
	}

	/* create a temporary file */
	fd1 = open("x", O_CREAT, 0777);
	if (fd1 == -1)
		tst_resm(TFAIL | TERRNO, "Could not open test file");
}

static void cleanup(void)
{
	close(fd1);

	tst_rmdir();
}
