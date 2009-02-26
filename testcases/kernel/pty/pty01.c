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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* 12/23/2002	Port to LTP	robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

/** LTP Port **/
#include "test.h"
#include "usctest.h"


char *TCID="pty01";            /* Test program identifier.    */
int TST_TOTAL=5;                /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */
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
static int
test1(void)
{
	int masterfd;		/* master pty fd */
	int slavefd;		/* slave pty fd */
	char *slavename;
	struct stat st;
	char buf[TESTSIZE];

	masterfd = open(MASTERCLONE, O_RDWR);
	if (masterfd < 0) {
		tst_resm(TBROK,"%s",MASTERCLONE);
		tst_exit();
	}

	slavename = ptsname(masterfd);
	if (slavename == (char *)0) {
		tst_resm(TBROK,"ptsname() call failed");
		tst_exit();
	}

	if (grantpt(masterfd) != 0) {
		tst_resm(TBROK,"grantpt() call failed");
		tst_exit();
	}

	if (stat(slavename, &st) != 0) {
		tst_resm(TBROK,"stat(%s) failed",slavename);
		tst_exit();
	}
	if (st.st_uid != getuid()) {
		tst_resm(TBROK, "uid mismatch");
		tst_exit();
	}

	if (st.st_mode != (S_IFCHR | S_IRUSR | S_IWUSR | S_IWGRP)) {
		tst_resm(TBROK, "mode mismatch");
		tst_resm(TBROK, "st.st_mode=%o",st.st_mode);
		tst_exit();
	}

	slavefd = open(slavename, O_RDWR);
	if (slavefd < 0) {
		/* expected */
	} else {
		tst_resm(TBROK, "open didn't fail!");
		tst_exit();
	}

	if (unlockpt(masterfd) != 0) {
		tst_resm(TBROK,"unlockpt() failed");
		tst_exit();
	}

	slavefd = open(slavename, O_RDWR);
	if (slavefd < 0) {
		tst_resm(TBROK,"Could not open %s",slavename);
		tst_exit();
	}

	/*
	 * test writing to the master / reading from the slave
	 */
	if (write(masterfd, STRING, strlen(STRING)) != strlen(STRING)) {
		tst_resm(TFAIL,"write to master");
		tst_exit();
	}

	if (read(slavefd, buf, strlen(STRING)) != strlen(STRING)) {
		tst_resm(TFAIL,"read from slave");
		tst_exit();
	}
	if (strncmp(STRING, buf,strlen(STRING)-1) != 0) {
		tst_resm(TFAIL, "strings are different.");
		tst_resm(TFAIL, "STRING:%s",STRING);
		tst_resm(TFAIL, "buf:%s",buf);
		tst_exit();
	}

	/*
	 * test writing to the slave / reading from the master
	 */
	if (write(slavefd, STRING, strlen(STRING)) != strlen(STRING)) {
		tst_resm(TFAIL,"write to slave");
		tst_exit();
	}

	if (read(masterfd, buf, strlen(STRING)) != strlen(STRING)) {
		tst_resm(TFAIL,"read from master");
		tst_exit();
	}
	if (strncmp(STRING, buf,strlen(STRING)-1) != 0) {
		tst_resm(TFAIL,"strings are different.");
		tst_resm(TFAIL, "STRING:%s",STRING);
		tst_resm(TFAIL, "buf:%s",buf);
		tst_exit();
	}

	/*
	 * try an invalid ioctl on the slave...
	 */
	if (ioctl(slavefd, TIOCGWINSZ, (char *)0) == 0)  {
		tst_resm(TFAIL, "invalid slave TIOCGWINSZ ioctl succeeded.. it should have failed");
		tst_exit();
	}

	/*
	 * try an invalid ioctl on the master...
	 */
	if (ioctl(masterfd, TIOCGWINSZ, (char *)0) == 0)  {
		tst_resm(TFAIL, "invalid master TIOCGWINSZ ioctl succeeded.. it should have failed");
		tst_exit();
	}

	/*
	 * close pty fds
	 */
	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close of slave");
		tst_exit();
	}
	if (close(masterfd) != 0) {
		tst_resm(TBROK,"close of master");
		tst_exit();
	}
	tst_resm(TPASS,"test1");
	/** NOT REACHED **/
	return 0;
}

/*
 * test slave operations with closed master
 */
static int
test2(void)
{
	int masterfd;		/* master pty fd */
	int slavefd;		/* slave pty fd */
	int i;
	char *slavename;
	char c;

	masterfd = open(MASTERCLONE, O_RDWR);
	if (masterfd < 0) {
		tst_resm(TBROK,"%s",MASTERCLONE);
		tst_exit();
	}

	slavename = ptsname(masterfd);
	if (slavename == (char *)0) {
		tst_resm(TBROK,"ptsname() call failed");
		tst_exit();
	}

	if (grantpt(masterfd) != 0) {
		tst_resm(TBROK,"grantpt() call failed");
		tst_exit();
	}

	if (unlockpt(masterfd) != 0) {
		tst_resm(TBROK,"unlockpt() call failed");
		tst_exit();
	}

	slavefd = open(slavename, O_RDWR);
	if (slavefd < 0) {
		tst_resm(TBROK,"Could not open %s",slavename);
		tst_exit();
	}

	/*
	 * close pty fds.  See what happens when we close the master
	 * first.
	 */
	if (close(masterfd) != 0) {
		tst_resm(TBROK,"close()");
		tst_exit();
	}

	errno = 0;
	if ((i = read(slavefd, &c, 1)) == 1) {
		tst_resm(TFAIL,"Try to read from slave (should return 0)");
		tst_resm(TFAIL, "read should have failed, but didn't");
		tst_resm(TFAIL, "read '%c'", c);
		tst_exit();
	}

	if ((i = write(slavefd, &c, 1)) == 1) {
		tst_resm(TFAIL,"try to write to slave (should fail)");
		tst_resm(TFAIL,"write should have failed, but didn't");
		tst_exit();
	}

	if (ioctl(slavefd, TIOCGWINSZ, (char *)0) == 0)  {
		tst_resm(TFAIL,"trying TIOCGWINSZ on slave (should fail)");
		tst_resm(TFAIL, "ioctl succeeded.. it should have failed");
		tst_exit();
	}

	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close");
		tst_exit();
	}
	tst_resm(TPASS,"test2");
	/** NOT REACHED **/
	return 0;
}

/*
 * test operations on master with closed slave
 */
static int
test3(void)
{
	int masterfd;		/* master pty fd */

	masterfd = open(MASTERCLONE, O_RDWR);
	if (masterfd < 0) {
		tst_resm(TBROK,"%s",MASTERCLONE);
		tst_exit();
	}

	if (ioctl(masterfd, TIOCGWINSZ, (char *)0) == 0)  {
		tst_resm(TFAIL,"trying TIOCGWINSZ on master with no open slave (should fail)");
		tst_resm(TFAIL,"ioctl succeeded.. it should have failed");
		tst_exit();
	}
	tst_resm(TPASS,"test3");
	/** NOT REACHED **/
	return 0;
}

/*
 * test multiple opens on slave side of pty
 */
static int
test4(void)
{
	int masterfd;		/* master pty fd */
	int slavefd;		/* slave pty fd */
	int slavefd2;
	int slavefd3;
	char *slavename;

	masterfd = open(MASTERCLONE, O_RDWR);
	if (masterfd < 0) {
		tst_resm(TBROK,"%s",MASTERCLONE);
		tst_exit();
	}

	slavename = ptsname(masterfd);
	if (slavename == (char *)0) {
		tst_resm(TBROK,"ptsname() call failed");
		tst_exit();
	}

	if (grantpt(masterfd) != 0) {
		tst_resm(TBROK,"grantpt() call failed");
		tst_exit();
	}

	if (unlockpt(masterfd) != 0) {
		tst_resm(TBROK,"unlockpt() call failed");
		tst_exit();
	}

	slavefd = open(slavename, O_RDWR);
	if (slavefd < 0) {
		tst_resm(TBROK,"Could not open %s",slavename);
		tst_exit();
	}

	slavefd2 = open(slavename, O_RDWR);
	if (slavefd < 0) {
		tst_resm(TFAIL,"Could not open %s (again)",slavename);
		tst_exit();
	}

	slavefd3 = open(slavename, O_RDWR);
	if (slavefd < 0) {
		tst_resm(TFAIL,"Could not open %s (once more)",slavename);
		tst_exit();
	}

	/*
	 * close pty fds.
	 */
	if (close(slavefd) != 0) {
		tst_resm(TBROK,"close slave");
		tst_exit();
	}

	if (close(slavefd2) != 0) {
		tst_resm(TBROK,"close slave again");
		tst_exit();
	}

	if (close(slavefd3) != 0) {
		tst_resm(TBROK,"close slave once more");
		tst_exit();
	}

	if (close(masterfd) != 0) {
		tst_resm(TBROK,"close master");
		tst_exit();
	}
	tst_resm(TPASS,"test4");
	/** NOT REACHED **/
	return 0;
}

/*
 * test opening/closing lots of ptys in parallel.  We may run out
 * of ptys for this test depending on how the system is configured,
 * but that's not a fatal error. 
 */
static int
test5(void)
{
	int masterfd;		/* master pty fd */
	char *slavename;
	int status;
	int i;

	for (i = 0; i < NUMPROCS; ++i) {
		switch (fork()) {
		case -1:
			tst_resm(TBROK,"fork()");
			break;
		case 0:
			masterfd = open(MASTERCLONE, O_RDWR);
			if (masterfd < 0) {
				tst_resm(TFAIL,"proc %d: %s",i,MASTERCLONE);
				tst_exit();
			}
			if (grantpt(masterfd) != 0) {
				tst_resm(TFAIL,"proc %d: grantpt() call failed",i);
				tst_exit();
			}
			slavename = ptsname(masterfd);
			if (slavename == (char *)0) {
				tst_resm(TFAIL,"proc %d: ptsname() call failed",i);
				tst_exit();
			}
			sleep(10);
			if (close(masterfd) != 0) {
				tst_resm(TFAIL,"proc %d: close",i);
				tst_exit();
			}
			tst_exit();
		default:
			break;
		}
	}
	while (wait(&status) > 0) {
		if (WEXITSTATUS(status) != 0) {
			tst_resm(TFAIL,"child exitted with non-zero status");
			tst_exit();
		}
	}
	tst_resm(TPASS,"test5");
	/** NOT REACHED **/
	return 0;
}
	
/*
 * main test driver
 */
int
main(int argc, char **argv)
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
	/*NOTREACHED*/
	return 0;
}
