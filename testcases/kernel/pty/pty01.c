/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* 12/23/2002	Port to LTP	robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/ioctl.h"

char *TCID = "pty01";		/* Test program identifier.    */
int TST_TOTAL = 5;		/* Total number of test cases. */
/**************/

/*
 * pty master clone device
 */
#define MASTERCLONE "/dev/ptmx"

/*
 * string for testing read/write on ptys
 */
#define STRING "Linux Test Project\n"

/*
 * test buffer size
 */
#define TESTSIZE 1024

/*
 * mode we expect grantpt() to leave pty as
 */
#define PTY_MODE 020622

/*
 * number of procs for parallel test
 */
#define NUMPROCS 15

/*
 * test slave locking
 */
static int test1(void)
{
	int masterfd;		/* master pty fd */
	int slavefd;		/* slave pty fd */
	char *slavename;
	struct stat st;
	char buf[TESTSIZE];

	masterfd = SAFE_OPEN(NULL, MASTERCLONE, O_RDWR);

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_brkm(TBROK | TERRNO, NULL, "ptsname() call failed");
	}

	if (grantpt(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "grantpt() call failed");
	}

	if (stat(slavename, &st) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "stat(%s) failed", slavename);
	}
	if (st.st_uid != getuid()) {
		tst_brkm(TBROK, NULL, "uid mismatch");
	}

	 /* grantpt() is a no-op in bionic. */
#ifndef __BIONIC__
	if (st.st_mode != (S_IFCHR | S_IRUSR | S_IWUSR | S_IWGRP)) {
		tst_brkm(TBROK, NULL, "mode mismatch (mode=%o)", st.st_mode);
	}
#endif

	slavefd = open(slavename, O_RDWR);
	if (slavefd >= 0) {
		tst_brkm(TBROK, NULL, "open didn't fail as expected!");
	}

	if (unlockpt(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "unlockpt() failed");
	}

	slavefd = SAFE_OPEN(NULL, slavename, O_RDWR);

	/*
	 * test writing to the master / reading from the slave
	 */
	if (write(masterfd, STRING, strlen(STRING)) != strlen(STRING)) {
		/*
		 * XXX: the errno printout might be garbage, but better to be
		 * safe than sorry..
		 */
		tst_brkm(TFAIL | TERRNO, NULL, "write to master");
	}

	if (read(slavefd, buf, strlen(STRING)) != strlen(STRING)) {
		/* XXX: Same as write above.. */
		tst_brkm(TFAIL | TERRNO, NULL, "read from slave");
	}
	if (strncmp(STRING, buf, strlen(STRING) - 1) != 0) {
		tst_brkm(TFAIL, NULL,
			 "strings are different (STRING = '%s' != buf = '%s')",
			 STRING, buf);
	}

	/*
	 * test writing to the slave / reading from the master
	 */
	if (write(slavefd, STRING, strlen(STRING)) != strlen(STRING)) {
		/* XXX: Same as write above.. */
		tst_brkm(TFAIL | TERRNO, NULL, "write to slave");
	}

	if (read(masterfd, buf, strlen(STRING)) != strlen(STRING)) {
		/* XXX: Same as write above.. */
		tst_brkm(TFAIL | TERRNO, NULL, "read from master");
	}
	if (strncmp(STRING, buf, strlen(STRING) - 1) != 0) {
		tst_brkm(TFAIL, NULL,
			 "strings are different (STRING = '%s' != buf = '%s').",
			 STRING, buf);
	}

	/*
	 * try an invalid ioctl on the slave...
	 */
	if (ioctl(slavefd, TIOCGWINSZ, NULL) == 0) {
		tst_brkm(TFAIL, NULL,
			 "invalid slave TIOCGWINSZ ioctl succeeded.. it should "
			 "have failed");
	}

	/*
	 * try an invalid ioctl on the master...
	 */
	if (ioctl(masterfd, TIOCGWINSZ, NULL) == 0) {
		tst_brkm(TFAIL, NULL,
			 "invalid master TIOCGWINSZ ioctl succeeded.. it should "
			 "have failed");
	}

	/*
	 * close pty fds
	 */
	if (close(slavefd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "close of slave");
	}
	if (close(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "close of master");
	}
	tst_resm(TPASS, "test1");
	/** NOTREACHED **/
	return 0;
}

/*
 * test slave operations with closed master
 */
static void test2(void)
{
	int masterfd;		/* master pty fd */
	int slavefd;		/* slave pty fd */
	int i;
	char *slavename;
	char c;

	masterfd = SAFE_OPEN(NULL, MASTERCLONE, O_RDWR);

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_brkm(TBROK | TERRNO, NULL, "ptsname() call failed");
	}

	if (grantpt(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "grantpt() call failed");
	}

	if (unlockpt(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "unlockpt() call failed");
	}

	slavefd = SAFE_OPEN(NULL, slavename, O_RDWR);

	/*
	 * close pty fds.  See what happens when we close the master
	 * first.
	 */
	if (close(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "close()");
	}

	errno = 0;
	if ((i = read(slavefd, &c, 1)) == 1) {
		tst_brkm(TFAIL, NULL,
			 "reading from slave fd should have failed, but didn't"
			 "(read '%c')", c);
	}

	if ((i = write(slavefd, &c, 1)) == 1) {
		tst_brkm(TFAIL, NULL,
			 "writing to slave fd should have failed, but didn't");
	}

	if (ioctl(slavefd, TIOCGWINSZ, NULL) == 0) {
		tst_brkm(TFAIL, NULL,
			 "trying TIOCGWINSZ on slave fd should have failed, "
			 "but didn't");
	}

	if (close(slavefd) != 0) {
		tst_brkm(TBROK, NULL, "close");
	}
	tst_resm(TPASS, "test2");
}

/*
 * test operations on master with closed slave
 */
static void test3(void)
{
	int masterfd;		/* master pty fd */

	masterfd = SAFE_OPEN(NULL, MASTERCLONE, O_RDWR);

	if (ioctl(masterfd, TIOCGWINSZ, NULL) == 0) {
		tst_brkm(TFAIL | TERRNO, NULL,
			 "trying TIOCGWINSZ on master with no open slave "
			 "succeeded unexpectedly");
	}
	tst_resm(TPASS, "test3");
}

/*
 * test multiple opens on slave side of pty
 */
static void test4(void)
{
	int masterfd;		/* master pty fd */
	int slavefd;		/* slave pty fd */
	int slavefd2;
	int slavefd3;
	char *slavename;

	masterfd = SAFE_OPEN(NULL, MASTERCLONE, O_RDWR);

	slavename = ptsname(masterfd);
	if (slavename == NULL) {
		tst_brkm(TBROK, NULL, "ptsname() call failed");
	}

	if (grantpt(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "grantpt() call failed");
	}

	if (unlockpt(masterfd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "unlockpt() call failed");
	}

	slavefd = SAFE_OPEN(NULL, slavename, O_RDWR);

	slavefd2 = open(slavename, O_RDWR);
	if (slavefd < 0) {
		tst_brkm(TFAIL | TERRNO, NULL, "Could not open %s (again)",
			 slavename);
	}

	slavefd3 = open(slavename, O_RDWR);
	if (slavefd < 0) {
		tst_brkm(TFAIL | TERRNO, NULL, "Could not open %s (once more)",
			 slavename);
	}

	/*
	 * close pty fds.
	 */
	if (close(slavefd) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "close slave");
	}

	if (close(slavefd2) != 0) {
		tst_brkm(TBROK, NULL, "close slave again");
	}

	if (close(slavefd3) != 0) {
		tst_brkm(TBROK, NULL, "close slave once more");
	}

	if (close(masterfd) != 0) {
		tst_brkm(TBROK, NULL, "close master");
	}
	tst_resm(TPASS, "test4");
}

/*
 * test opening/closing lots of ptys in parallel.  We may run out
 * of ptys for this test depending on how the system is configured,
 * but that's not a fatal error.
 */
static void test5(void)
{
	int masterfd;		/* master pty fd */
	char *slavename;
	int status;
	int i;

	for (i = 0; i < NUMPROCS; ++i) {
		switch (fork()) {
		case -1:
			tst_brkm(TBROK, NULL, "fork()");
			break;
		case 0:
			masterfd = open(MASTERCLONE, O_RDWR);
			if (masterfd < 0) {
				printf("proc %d: opening %s failed: %s",
				       i, MASTERCLONE, strerror(errno));
				exit(1);
			}
			if (grantpt(masterfd) != 0) {
				printf("proc %d: grantpt() call failed: %s",
				       i, strerror(errno));
				exit(1);
			}
			slavename = ptsname(masterfd);
			if (slavename == NULL) {
				printf("proc %d: ptsname() call failed: %s",
				       i, strerror(errno));
				exit(1);
			}
			sleep(10);
			if (close(masterfd) != 0) {
				printf("proc %d: close failed: %s",
				       i, strerror(errno));
				exit(1);
			}
			exit(0);
		default:
			break;
		}
	}
	while (wait(&status) > 0) {
		if (status) {
			tst_brkm(TFAIL, NULL,
				 "child exited with non-zero status %d",
				 status);
		}
	}
	tst_resm(TPASS, "test5");
}

/*
 * main test driver
 */
int main(int argc, char **argv)
{
	test1();
	test2();
	test3();
	test4();
	test5();

	/*
	 * all done
	 */
	tst_exit();
}
