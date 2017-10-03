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
 *	fcntl14.c
 *
 * DESCRIPTION
 *	File locking test cases for fcntl. In Linux, S_ENFMT is not implemented
 *	in the kernel. However all standard Unix kernels define S_ENFMT as
 *	S_ISGID. So this test defines S_ENFMT as S_ISGID.
 *
 * ALGORITHM
 *	Various test cases are used to lock a file opened without mandatory
 *	locking, with mandatory locking and mandatory locking with NOBLOCK
 *
 * USAGE
 *	fcntl14
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */
#define _GNU_SOURCE 1
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <inttypes.h>
#include "test.h"
#include "safe_macros.h"

#define SKIP 0x0c00
#if SKIP == F_RDLCK || SKIP== F_WRLCK
#error invalid value for SKIP, must be distinct from F_RDLCK and F_WRLCK
#endif
#ifndef S_ENFMT
#define S_ENFMT S_ISGID
#endif

/* NOBLOCK - immediate success */
#define NOBLOCK 2

/* WILLBLOCK - blocks, then succeeds (parent must unlock records) */
#define WILLBLOCK 3

#define TIME_OUT 60

typedef struct {
	short a_type;
	short a_whence;
	long a_start;
	long a_len;
	short b_type;		/* SKIP means suppress fcntl call */
	short b_whence;
	long b_start;
	long b_len;
	short c_type;
	int c_whence;
	long c_start;
	long c_len;
	short c_flag;
} testcase;

static testcase testcases[] = {
	/* Test cases: entire boundary */
	/* #1 Parent making a write lock on entire file */
	{F_WRLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock on entire file */
	 F_RDLCK, 0, 0L, 0L, WILLBLOCK},

	/* #2 Parent making a write lock on entire file */
	{F_WRLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock on entire file */
	 F_WRLCK, 0, 0L, 0L, WILLBLOCK},

	/* #3 Parent making a read lock on entire file */
	{F_RDLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock on entire file */
	 F_RDLCK, 0, 0L, 0L, NOBLOCK},

	/* #4 Parent making a read lock on entire file */
	{F_RDLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock on entire file */
	 F_WRLCK, 0, 0L, 0L, WILLBLOCK},

	/* Test case: start boundary */
	/* #5 Parent making a write lock on entire file */
	{F_WRLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /*
	  * Child attempting a read lock from beginning of
	  * file for 5 bytes
	  */
	 F_RDLCK, 0, 0L, 5L, WILLBLOCK},

	/* #6 Parent making a write lock on entire file */
	{F_WRLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /*
	  * Child attempting a write lock from beginning of
	  * file for 5 bytes
	  */
	 F_WRLCK, 0, 0L, 5L, WILLBLOCK},

	/* #7 Parent making a read lock on entire file */
	{F_RDLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /*
	  * Child attempting a read lock from beginning of
	  * file for 5 bytes
	  */
	 F_RDLCK, 0, 0L, 5L, NOBLOCK},

	/* #8 Parent making a read lock on entire file */
	{F_RDLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /*
	  * Child attempting a write lock from beginning of
	  * file for 5 bytes
	  */
	 F_WRLCK, 0, 0L, 5L, WILLBLOCK},

	/* Test cases: end boundary */
	/* #9 Parent making a write lock on entire file */
	{F_WRLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from byte 7 to end of file */
	 F_RDLCK, 0, 7L, 0L, WILLBLOCK},

	/* #10 Parent making a write lock on entire file */
	{F_WRLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from byte 7 to end of file */
	 F_WRLCK, 0, 7L, 0L, WILLBLOCK},

	/* #11 Parent making a read lock on entire file */
	{F_RDLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from byte 7 to end of file */
	 F_RDLCK, 0, 7L, 0L, NOBLOCK},

	/* #12 Parent making a read lock on entire file */
	{F_RDLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from byte 7 to end of file */
	 F_WRLCK, 0, 7L, 0L, WILLBLOCK},

	/* Test cases: entire boundary ( less than entire file) */
	/*
	 * #13 Parent making a write lock from beginning of
	 * file for 5 bytes
	 */
	{F_WRLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /*
	  * Child attempting a read lock from beginning of
	  * file for 5 bytes
	  */
	 F_RDLCK, 0, 0L, 5L, WILLBLOCK},

	/*
	 * #14 Parent making a write lock from beginning of file
	 * for 5 bytes
	 */
	{F_WRLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /*
	  * Child attempting a write lock from beginning of
	  * file for 5 bytes
	  */
	 F_WRLCK, 0, 0L, 5L, WILLBLOCK},

	/*
	 * #15 Parent making a read lock from beginning of
	 * file for 5 bytes
	 */
	{F_RDLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /*
	  * Child attempting a read lock from beginning of
	  * file for 5 bytes
	  */
	 F_RDLCK, 0, 0L, 5L, NOBLOCK},

	/*
	 * #16 Parent making a read lock from beginning of
	 * file for 5 bytes
	 */
	{F_RDLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /*
	  * Child attempting a write lock from beginning
	  * of file for 5 bytes
	  */
	 F_WRLCK, 0, 0L, 5L, WILLBLOCK},

	/* Test cases: inside boundary */
	/*
	 * #17 Parent making a write lock from beginning
	 * of file for 5 bytes
	 */
	{F_WRLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from byte 2 to byte 4 */
	 F_RDLCK, 0, 1L, 3L, WILLBLOCK},

	/*
	 * #18 Parent making a write lock from beginning of
	 * file for 5 bytes
	 */
	{F_WRLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from byte 2 to byte 4 */
	 F_WRLCK, 0, 1L, 3L, WILLBLOCK},

	/*
	 * #19 Parent making a read lock from beginning of
	 * file for 5 bytes
	 */
	{F_RDLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from byte 2 to byte 4 */
	 F_RDLCK, 0, 1L, 3L, NOBLOCK},

	/*
	 * #20 Parent making a read lock from beginning of
	 * file for 5 bytes
	 */
	{F_RDLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from byte 2 to byte 4 */
	 F_WRLCK, 0, 1L, 3L, WILLBLOCK},

	/* Test cases: cross boundary (inside to after) */
	/*
	 * #21 Parent making a write lock from beginning of
	 * file for 5 bytes
	 */
	{F_WRLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from byte 3 to byte 7 */
	 F_RDLCK, 0, 2L, 5L, WILLBLOCK},

	/*
	 * #22 Parent making a write lock from beginning
	 * of file for 5 bytes
	 */
	{F_WRLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from byte 3 to byte 7 */
	 F_WRLCK, 0, 2L, 5L, WILLBLOCK},

	/*
	 * #23 Parent making a read lock from beginning of
	 * file for 5 bytes
	 */
	{F_RDLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from byte 3 to byte 7 */
	 F_RDLCK, 0, 2L, 5L, NOBLOCK},

	/*
	 * #24 Parent making a read lock from beginning of
	 * file for 5 bytes
	 */
	{F_RDLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from byte 3 to byte 7 */
	 F_WRLCK, 0, 2L, 5L, WILLBLOCK},

	/* Test cases: outside boundary (after) */

	/*
	 * #25 Parent making a write lock from beginning of
	 * file for 5 bytes
	 */
	{F_WRLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /*  Child attempting a read lock from byte 7 to end of file */
	 F_RDLCK, 0, 6L, 0L, NOBLOCK},

	/*
	 * #26 Parent making a write lock from beginning of
	 * file for 5 bytes
	 */
	{F_WRLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from byte 7 to end of file */
	 F_WRLCK, 0, 6L, 0L, NOBLOCK},

	/*
	 * #27 Parent making a read lock from beginning of
	 * file for 5 bytes
	 */
	{F_RDLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from byte 7 to end of file */
	 F_RDLCK, 0, 6L, 0L, NOBLOCK},

	/*
	 * #28 Parent making a read lock from beginning of
	 * file for 5 bytes
	 */
	{F_RDLCK, 0, 0L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from byte 7 to end of file */
	 F_WRLCK, 0, 6L, 0L, NOBLOCK},

	/* Test cases: outside boundary (before) */

	/* #29 Parent making a write lock from byte 3 to byte 7 */
	{F_WRLCK, 0, 2L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from beginning of file to byte 2 */
	 F_RDLCK, 0, 0L, 2L, NOBLOCK},

	/* #30 Parent making a write lock from byte 3 to byte 7 */
	{F_WRLCK, 0, 2L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from beginning of file to byte 2 */
	 F_WRLCK, 0, 0L, 2L, NOBLOCK},

	/* #31 Parent making a write lock from byte 3 to byte 7 */
	{F_RDLCK, 0, 2L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from beginning of file to byte 2 */
	 F_RDLCK, 0, 0L, 2L, NOBLOCK},

	/* #32 Parent making a write lock from byte 3 to byte 7 */
	{F_RDLCK, 0, 2L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from beginning of file to byte 2 */
	 F_WRLCK, 0, 0L, 2L, NOBLOCK},

	/* Test cases: cross boundary (before to inside) */
	/* #33 Parent making a write lock from byte 5 to end of file */
	{F_WRLCK, 0, 4L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from byte 3 to byte 7 */
	 F_RDLCK, 0, 2L, 5L, WILLBLOCK},

	/* #34 Parent making a write lock from byte 5 to end of file */
	{F_WRLCK, 0, 4L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from byte 3 to byte 7 */
	 F_WRLCK, 0, 2L, 5L, WILLBLOCK},

	/* #35 Parent making a read lock from byte 5 to end of file */
	{F_RDLCK, 0, 4L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a read lock from byte 3 to byte 7 */
	 F_RDLCK, 0, 2L, 5L, NOBLOCK},

	/* #36 Parent making a read lock from byte 5 to end of file */
	{F_RDLCK, 0, 4L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting a write lock from byte 3 to byte 7 */
	 F_WRLCK, 0, 2L, 5L, WILLBLOCK},

	/* Start of negative L_start and L_len test cases */
	/*
	 * #37 Parent making write lock from byte 2 to byte 3
	 * with L_start = -3
	 */
	{F_WRLCK, 1, -3L, 2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 1 */
	 F_WRLCK, 0, 1L, 1L, NOBLOCK},

	/*
	 * #38 Parent making write lock from byte 2 to byte 3
	 * with L_start = -3
	 */
	{F_WRLCK, 1, -3L, 2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 4 */
	 F_WRLCK, 0, 4L, 1L, NOBLOCK},

	/*
	 * #39 Parent making write lock from byte 2 to byte 3
	 * with L_start = -3
	 */
	{F_WRLCK, 1, -3L, 2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 2 */
	 F_WRLCK, 0, 2L, 1L, WILLBLOCK},

	/*
	 * #40 Parent making write lock from byte 2 to byte 3
	 * with L_start = -3
	 */
	{F_WRLCK, 1, -3L, 2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 3 */
	 F_WRLCK, 0, 3L, 1L, WILLBLOCK},

	/*
	 * #41 Parent making write lock from byte 2 to byte 6
	 * with L_start = -3
	 */
	{F_WRLCK, 1, -3L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 1 */
	 F_WRLCK, 0, 1L, 1L, NOBLOCK},

	/*
	 * #42 Parent making write lock from byte 2 to byte 6
	 * with L_start = -3
	 */
	{F_WRLCK, 1, -3L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 7 */
	 F_WRLCK, 0, 1L, 1L, NOBLOCK},

	/*
	 * #43 Parent making write lock from byte 2 to byte 6
	 * with L_start = -3
	 */
	{F_WRLCK, 1, -3L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 2 */
	 F_WRLCK, 0, 2L, 1L, WILLBLOCK},

	/*
	 * #44 Parent making write lock from byte 2 to byte 6
	 * with L_start = -3
	 */
	{F_WRLCK, 1, -3L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 5 */
	 F_WRLCK, 0, 5L, 1L, WILLBLOCK},

	/*
	 * #45 Parent making write lock from byte 2 to byte 6
	 * with L_start = -3
	 */
	{F_WRLCK, 1, -3L, 5L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 6 */
	 F_WRLCK, 0, 6L, 1L, WILLBLOCK},

	/*
	 * #46 Parent making write lock from byte 2 to byte 3 with
	 * L_start = -2 and L_len = -2
	 */
	{F_WRLCK, 1, 2L, -2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 1 */
	 F_WRLCK, 0, 1L, 1L, NOBLOCK},

	/*
	 * #47 Parent making write lock from byte 2 to byte 3 with
	 * L_start = -2 and L_len = -2
	 */
	{F_WRLCK, 1, -2L, -2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 4 */
	 F_WRLCK, 0, 4L, 1L, NOBLOCK},

	/*
	 * #48 Parent making write lock from byte 2 to byte 3 with
	 * L_start = -2 and L_len = -2
	 */
	{F_WRLCK, 1, -2L, -2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 2 */
	 F_WRLCK, 0, 2L, 1L, WILLBLOCK},

	/*
	 * #49 Parent making write lock from byte 2 to byte 3 with
	 * L_start = -2 and L_len = -2
	 */
	{F_WRLCK, 1, -2L, -2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 3 */
	 F_WRLCK, 0, 3L, 1L, WILLBLOCK},

	/*
	 * #50 Parent making write lock from byte 6 to byte 7 with
	 * L_start = 2 and L_len = -2
	 */
	{F_WRLCK, 1, 2L, -2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 5 */
	 F_WRLCK, 0, 5L, 1L, NOBLOCK},

	/*
	 * #51 Parent making write lock from byte 6 to byte 7 with
	 * L_start = 2 and L_len = -2
	 */
	{F_WRLCK, 1, 2L, -2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 8 */
	 F_WRLCK, 0, 8L, 1L, NOBLOCK},

	/*
	 * #52 Parent making write lock from byte 6 to byte 7 with
	 * L_start = 2 and L_len = -2
	 */
	{F_WRLCK, 1, 2L, -2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 6 */
	 F_WRLCK, 0, 6L, 1L, WILLBLOCK},

	/*
	 * #53 Parent making write lock from byte 6 to byte 7 with
	 * L_start = 2 and L_len = -2
	 */
	{F_WRLCK, 1, 2L, -2L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 7 */
	 F_WRLCK, 0, 7L, 1L, WILLBLOCK},

	/*
	 * #54 Parent making write lock from byte 3 to byte 7 with
	 * L_start = 2 and L_len = -5
	 */
	{F_WRLCK, 1, 2L, -5L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 2 */
	 F_WRLCK, 0, 2L, 1L, NOBLOCK},

	/*
	 * #55 Parent making write lock from byte 3 to byte 7 with
	 * L_start = 2 and L_len = -5
	 */
	{F_WRLCK, 1, 2L, -5L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 8 */
	 F_WRLCK, 0, 8L, 1L, NOBLOCK},

	/*
	 * #56 Parent making write lock from byte 3 to byte 7 with
	 * L_start = 2 and L_len = -5
	 */
	{F_WRLCK, 1, 2L, -5L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 3 */
	 F_WRLCK, 0, 3L, 1L, WILLBLOCK},

	/*
	 * #57 Parent making write lock from byte 3 to byte 7 with
	 * L_start = 2 and L_len = -5
	 */
	{F_WRLCK, 1, 2L, -5L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 5 */
	 F_WRLCK, 0, 5L, 1L, WILLBLOCK},

	/*
	 * #58 Parent making write lock from byte 3 to byte 7 with
	 * L_start = 2 and L_len = -5
	 */
	{F_WRLCK, 1, 2L, -5L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 7 */
	 F_WRLCK, 0, 7L, 1L, WILLBLOCK},

	/* Test case for block 4 */
	/* #59 Parent making write lock on entire file */
	{F_WRLCK, 0, 0L, 0L, SKIP, 0, 0L, 0L,
	 /* Child attempting write lock on byte 15 to end of file */
	 F_WRLCK, 0, 15L, 0L, WILLBLOCK},
};

static testcase *thiscase;
static struct flock flock;
static int parent, child, status, fail = 0;
static int got1 = 0;
static int fd;
static int test;
static char tmpname[40];

#define FILEDATA	"ten bytes!"

void catch1(int sig);
void catch_alarm(int sig);

char *TCID = "fcntl14";
int TST_TOTAL = 1;
int NO_NFS = 1;

#ifdef UCLINUX
static char *argv0;
#endif

void cleanup(void)
{
	tst_rmdir();
}

void setup(void)
{
	struct sigaction act;

	tst_sig(FORK, DEF_HANDLER, cleanup);
	signal(SIGHUP, SIG_IGN);
	umask(0);
	TEST_PAUSE;
	tst_tmpdir();
	parent = getpid();

	sprintf(tmpname, "fcntl2.%d", parent);

	/* setup signal handler for signal from child */
	memset(&act, 0, sizeof(act));
	act.sa_handler = catch1;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGUSR1);
	if ((sigaction(SIGUSR1, &act, NULL)) < 0) {
		tst_resm(TFAIL, "SIGUSR1 signal setup failed, errno = %d",
			 errno);
		cleanup();
	}

	memset(&act, 0, sizeof(act));
	act.sa_handler = catch_alarm;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGALRM);
	if ((sigaction(SIGALRM, &act, NULL)) < 0) {
		tst_resm(TFAIL, "SIGALRM signal setup failed");
		cleanup();
	}
}

void wake_parent(void)
{
	if ((kill(parent, SIGUSR1)) < 0) {
		tst_resm(TFAIL, "Attempt to send signal to parent " "failed");
		tst_resm(TFAIL, "Test case %d, errno = %d", test + 1, errno);
		fail = 1;
	}
}

void do_usleep_child(void)
{
	usleep(100000);		/* XXX how long is long enough? */
	wake_parent();
	exit(0);
}

void dochild(void)
{
	int rc;
	pid_t pid;

	flock.l_type = thiscase->c_type;
	flock.l_whence = thiscase->c_whence;
	flock.l_start = thiscase->c_start;
	flock.l_len = thiscase->c_len;
	flock.l_pid = 0;
	fail = 0;

	/*
	 * Check to see if child lock will succeed. If it will, FLOCK
	 * structure will return with l_type changed to F_UNLCK. If it will
	 * not, the parent pid will be returned in l_pid and the type of
	 * lock that will block it in l_type.
	 */
	if ((rc = fcntl(fd, F_GETLK, &flock)) < 0) {
		tst_resm(TFAIL, "Attempt to check lock status failed");
		tst_resm(TFAIL, "Test case %d, errno = %d", test + 1, errno);
		fail = 1;
	} else {

		if ((thiscase->c_flag) == NOBLOCK) {
			if (flock.l_type != F_UNLCK) {
				tst_resm(TFAIL,
					 "Test case %d, GETLK: type = %d, "
					 "%d was expected", test + 1,
					 flock.l_type, F_UNLCK);
				fail = 1;
			}

			if (flock.l_whence != thiscase->c_whence) {
				tst_resm(TFAIL,
					 "Test case %d, GETLK: whence = %d, "
					 "should have remained %d", test + 1,
					 flock.l_whence, thiscase->c_whence);
				fail = 1;
			}

			if (flock.l_start != thiscase->c_start) {
				tst_resm(TFAIL,
					 "Test case %d, GETLK: start = %" PRId64
					 ", " "should have remained %" PRId64,
					 test + 1, (int64_t) flock.l_start,
					 (int64_t) thiscase->c_start);
				fail = 1;
			}

			if (flock.l_len != thiscase->c_len) {
				tst_resm(TFAIL,
					 "Test case %d, GETLK: len = %" PRId64
					 ", " "should have remained %" PRId64,
					 test + 1, (int64_t) flock.l_len,
					 (int64_t) thiscase->c_len);
				fail = 1;
			}

			if (flock.l_pid != 0) {
				tst_resm(TFAIL,
					 "Test case %d, GETLK: pid = %d, "
					 "should have remained 0", test + 1,
					 flock.l_pid);
				fail = 1;
			}
		} else {
			if (flock.l_pid != parent) {
				tst_resm(TFAIL,
					 "Test case %d, GETLK: pid = %d, "
					 "should be parent's id of %d",
					 test + 1, flock.l_pid, parent);
				fail = 1;
			}

			if (flock.l_type != thiscase->a_type) {
				tst_resm(TFAIL,
					 "Test case %d, GETLK: type = %d, "
					 "should be parent's first lock type of %d",
					 test + 1, flock.l_type,
					 thiscase->a_type);
				fail = 1;
			}
		}
	}

	/*
	 * now try to set the lock, nonblocking
	 * This will succeed for NOBLOCK,
	 * fail for WILLBLOCK
	 */
	flock.l_type = thiscase->c_type;
	flock.l_whence = thiscase->c_whence;
	flock.l_start = thiscase->c_start;
	flock.l_len = thiscase->c_len;
	flock.l_pid = 0;

	if ((rc = fcntl(fd, F_SETLK, &flock)) < 0) {
		if ((thiscase->c_flag) == NOBLOCK) {
			tst_resm(TFAIL, "Attempt to set child NONBLOCKING "
				 "lock failed");
			tst_resm(TFAIL, "Test case %d, errno = %d",
				 test + 1, errno);
			fail = 1;
		}
	}

	if ((thiscase->c_flag) == WILLBLOCK) {
		if (rc != -1 || (errno != EACCES && errno != EAGAIN)) {
			tst_resm(TFAIL,
				 "SETLK: rc = %d, errno = %d, -1/EAGAIN or EACCES "
				 "was expected", rc, errno);
			fail = 1;
		}
		if (rc == 0) {
			/* accidentally got the lock */
			/* XXX how to clean up? */
			(void)fcntl(fd, F_UNLCK, &flock);
		}
		/*
		 * Lock should succeed after blocking and parent releases
		 * lock, tell the parent to release the locks.
		 * Do the lock in this process, send the signal in a child
		 * process, so that the SETLKW actually uses the blocking
		 * mechanism in the kernel.
		 *
		 * XXX inherent race: we want to wait until the
		 * F_SETLKW has started, but we don't have a way to
		 * check that reliably in the child.  (We'd
		 * need some way to have fcntl() atomically unblock a
		 * signal and wait for the lock.)
		 */
		pid = FORK_OR_VFORK();
		switch (pid) {
		case -1:
			tst_resm(TFAIL, "Fork failed");
			fail = 1;
			break;
		case 0:
#ifdef UCLINUX
			if (self_exec(argv0, "nd", 1, parent) < 0) {
				tst_resm(TFAIL, "self_exec failed");
				break;
			}
#else
			do_usleep_child();
#endif
			break;

		default:
			if ((rc = fcntl(fd, F_SETLKW, &flock)) < 0) {
				tst_resm(TFAIL, "Attempt to set child BLOCKING "
					 "lock failed");
				tst_resm(TFAIL, "Test case %d, errno = %d",
					 test + 1, errno);
				fail = 1;
			}
			waitpid(pid, &status, 0);
			break;
		}
	}
	if (fail) {
		exit(1);
	} else {
		exit(0);
	}
}

void run_test(int file_flag, int file_mode, int seek, int start, int end)
{
	fail = 0;

	for (test = start; test < end; test++) {
		fd = SAFE_OPEN(cleanup, tmpname, file_flag, file_mode);

		if (write(fd, FILEDATA, 10) < 0)
			tst_brkm(TBROK, cleanup, "write() failed");

		if (seek) {
			SAFE_LSEEK(cleanup, fd, seek, 0);
		}

		thiscase = &testcases[test];
		flock.l_type = thiscase->a_type;
		flock.l_whence = thiscase->a_whence;
		flock.l_start = thiscase->a_start;
		flock.l_len = thiscase->a_len;

		/* set the initial parent lock on the file */
		if ((fcntl(fd, F_SETLK, &flock)) < 0) {
			tst_resm(TFAIL, "First parent lock failed");
			tst_resm(TFAIL, "Test case %d, errno = %d",
				 test + 1, errno);
			fail = 1;
		}

		if ((thiscase->b_type) != SKIP) {
			flock.l_type = thiscase->b_type;
			flock.l_whence = thiscase->b_whence;
			flock.l_start = thiscase->b_start;
			flock.l_len = thiscase->b_len;

			/* set the second parent lock */
			if ((fcntl(fd, F_SETLK, &flock)) < 0) {
				tst_resm(TFAIL, "Second parent lock failed");
				tst_resm(TFAIL, "Test case %d, errno = %d",
					 test + 1, errno);
				fail = 1;
			}
		}
		if ((thiscase->c_type) == SKIP) {
			close(fd);
			tst_resm(TINFO, "skipping test %d", test + 1);
			continue;
		}

		/* Mask SIG_USR1 before forking child, to avoid race */
		(void)sighold(SIGUSR1);

		/* flush the stdout to avoid garbled output */
		fflush(stdout);

		if ((child = FORK_OR_VFORK()) == 0) {
#ifdef UCLINUX
			if (self_exec(argv0, "nddddddddd", 2, thiscase->c_type,
				      thiscase->c_whence, thiscase->c_start,
				      thiscase->c_len, thiscase->c_flag,
				      thiscase->a_type, fd, test, parent) < 0) {
				tst_resm(TFAIL, "self_exec failed");
				cleanup();
			}
#else
			dochild();
#endif
		}
		if (child < 0)
			tst_brkm(TBROK|TERRNO, cleanup, "Fork failed");

		if ((thiscase->c_flag) == WILLBLOCK) {
			/*
			 * Wait for a signal from the child then remove
			 * blocking lock. Set a 60 second alarm to break the
			 * pause just in case the child never signals us.
			 */
			alarm(TIME_OUT);
			sigpause(SIGUSR1);

			/* turn off the alarm timer */
			alarm((unsigned)0);
			if (got1 != 1)
				tst_resm(TINFO, "Pause terminated without "
					 "signal SIGUSR1 from child");
			got1 = 0;

			/*
			 * setup lock structure for parent to delete
			 * blocking lock then wait for child to exit
			 */
			flock.l_type = F_UNLCK;
			flock.l_whence = 0;
			flock.l_start = 0L;
			flock.l_len = 0L;
			if ((fcntl(fd, F_SETLK, &flock)) < 0) {
				tst_resm(TFAIL, "Attempt to release parent "
					 "lock failed");
				tst_resm(TFAIL, "Test case %d, errno = %d",
					 test + 1, errno);
				fail = 1;
			}
		}
		/*
		 * set a 60 second alarm to break the wait just in case the
		 * child doesn't terminate on its own accord
		 */
		alarm(TIME_OUT);

		/* wait for the child to terminate and close the file */
		waitpid(child, &status, 0);
		/* turn off the alarm clock */
		alarm((unsigned)0);
		if (status != 0) {
			tst_resm(TFAIL, "tchild returned status 0x%x", status);
			fail = 1;
		}
		close(fd);
		if (fail)
			tst_resm(TFAIL, "testcase:%d FAILED", test + 1);
		else
			tst_resm(TPASS, "testcase:%d PASSED", test + 1);
	}
	unlink(tmpname);
}

void catch_alarm(int sig)
{
	/*
	 * Timer has runout and child has not signaled, need
	 * to kill off the child as it appears it will not
	 * on its own accord. Check that it is still around
	 * as it may have terminated abnormally while parent
	 * was waiting for SIGUSR1 signal from it.
	 */
	if (kill(child, 0) == 0) {
		kill(child, SIGKILL);
		perror("The child didnot terminate on its own accord");
	}
}

void catch1(int sig)
{
	struct sigaction act;

	/*
	 * Set flag to let parent know that child is ready to have lock
	 * removed
	 */
	memset(&act, 0, sizeof(act));
	act.sa_handler = catch1;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGUSR1);
	sigaction(SIGUSR1, &act, NULL);
	got1++;
}

static void testcheck_end(int check_fail, char *msg)
{
	if (check_fail)
		tst_resm(TFAIL, "%s FAILED", msg);
	else
		tst_resm(TPASS, "%s PASSED", msg);
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	argv0 = av[0];

	maybe_run_child(&do_usleep_child, "nd", 1, &parent);
	thiscase = malloc(sizeof(testcase));

	maybe_run_child(&dochild, "nddddddddd", 2, &thiscase->c_type,
			&thiscase->c_whence, &thiscase->c_start,
			&thiscase->c_len, &thiscase->c_flag, &thiscase->a_type,
			&fd, &test, &parent);
#endif

	setup();

	if (tst_fs_type(cleanup, ".") == TST_NFS_MAGIC)
		NO_NFS = 0;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

/* //block1: */
		tst_resm(TINFO, "Enter block 1: without mandatory locking");
		fail = 0;
		/*
		 * try various file locks on an ordinary file without
		 * mandatory locking
		 */
		(void)run_test(O_CREAT | O_RDWR | O_TRUNC, 0777, 0, 0, 36);
		testcheck_end(fail, "Block 1, test 1");

		/* Now try with negative values for L_start and L_len */
		(void)run_test(O_CREAT | O_RDWR | O_TRUNC, 0777, 5, 36, 45);
		testcheck_end(fail, "Block 1, test 2");

		tst_resm(TINFO, "Exit block 1");

/* //block2: */
		/*
		 * Skip block2 if test on NFS, since NFS does not support
		 * mandatory locking
		 */
		tst_resm(TINFO, "Enter block 2: with mandatory locking");
		if (NO_NFS) {
			fail = 0;
			/*
			 * Try various locks on a file with mandatory
			 * record locking this should behave the same
			 * as an ordinary file
			 */
			(void)run_test(O_CREAT | O_RDWR | O_TRUNC,
				S_ENFMT | S_IRUSR | S_IWUSR, 0, 0, 36);
			testcheck_end(fail, "Block 2, test 1");

			/* Now try negative values for L_start and L_len */
			(void)run_test(O_CREAT | O_RDWR | O_TRUNC,
				S_ENFMT | S_IRUSR | S_IWUSR, 5, 36, 45);
			testcheck_end(fail, "Block 2, test 2");
		} else {
			tst_resm(TCONF, "Skip block 2 as NFS does not"
				" support mandatory locking");
		}

		tst_resm(TINFO, "Exit block 2");

/* //block3: */
		tst_resm(TINFO, "Enter block 3");
		fail = 0;
		/*
		 * Check that proper error status is returned when invalid
		 * argument used for WHENCE (negative value)
		 */

		fd = SAFE_OPEN(cleanup, tmpname, O_CREAT | O_RDWR | O_TRUNC,
			       0777);

		if (write(fd, FILEDATA, 10) < 0)
			tst_brkm(TBROK, cleanup, "write failed");

		flock.l_type = F_WRLCK;
		flock.l_whence = -1;
		flock.l_start = 0L;
		flock.l_len = 0L;

		if ((fcntl(fd, F_SETLK, &flock)) < 0) {
			if (errno != EINVAL) {
				tst_resm(TFAIL, "Expected %d got %d",
					 EINVAL, errno);
				fail = 1;
			}
		} else {
			tst_resm(TFAIL, "Lock succeeded when it should have "
				 "failed");
			fail = 1;
		}

		close(fd);
		unlink(tmpname);

		testcheck_end(fail, "Test with negative whence locking");
		tst_resm(TINFO, "Exit block 3");

/* //block4: */
		tst_resm(TINFO, "Enter block 4");
		fail = 0;
		/*
		 * Check that a lock on end of file is still valid when
		 * additional data is appended to end of file and a new
		 * process attempts to lock new data
		 */
		fd = SAFE_OPEN(cleanup, tmpname, O_CREAT | O_RDWR | O_TRUNC,
			       0777);

		if (write(fd, FILEDATA, 10) < 0)
			tst_brkm(TBROK, cleanup, "write failed");

		thiscase = &testcases[58];
		flock.l_type = thiscase->a_type;
		flock.l_whence = thiscase->a_whence;
		flock.l_start = thiscase->a_start;
		flock.l_len = thiscase->a_len;

		/* Set the initial parent lock on the file */
		if ((fcntl(fd, F_SETLK, &flock)) < 0) {
			tst_resm(TFAIL, "First parent lock failed");
			tst_resm(TFAIL, "Test case %d, errno = %d", 58, errno);
			fail = 1;
		}

		/* Write some additional data to end of file */
		if (write(fd, FILEDATA, 10) < 0)
			tst_brkm(TBROK, cleanup, "write failed");

		/* Mask signal to avoid race */
		if (sighold(SIGUSR1) < 0)
			tst_brkm(TBROK, cleanup, "sighold failed");

		if ((child = FORK_OR_VFORK()) == 0) {
#ifdef UCLINUX
			if (self_exec(argv0, "nddddddddd", 2, thiscase->c_type,
				      thiscase->c_whence, thiscase->c_start,
				      thiscase->c_len, thiscase->c_flag,
				      thiscase->a_type, fd, test, parent) < 0) {
				tst_resm(TFAIL, "self_exec failed");
				cleanup();
			}
#else
			dochild();
#endif
		}
		if (child < 0)
			tst_brkm(TBROK|TERRNO, cleanup, "Fork failed");

		/*
		 * Wait for a signal from the child then remove blocking lock.
		 * Set a 60 sec alarm to break the pause just in case the
		 * child doesn't terminate on its own accord
		 */
		(void)alarm(TIME_OUT);

		(void)sigpause(SIGUSR1);

		/* turn off the alarm timer */
		(void)alarm((unsigned)0);
		if (got1 != 1) {
			tst_resm(TINFO, "Pause terminated without signal "
				 "SIGUSR1 from child");
		}
		got1 = 0;

		/*
		 * Set up lock structure for parent to delete
		 * blocking lock then wait for child to exit
		 */
		flock.l_type = F_UNLCK;
		flock.l_whence = 0;
		flock.l_start = 0L;
		flock.l_len = 0L;
		if ((fcntl(fd, F_SETLK, &flock)) < 0) {
			tst_resm(TFAIL, "Attempt to release parent lock "
				 "failed");
			tst_resm(TFAIL, "Test case %d, errno = %d", test + 1,
				 errno);
			fail = 1;
		}

		/*
		 * set a 60 sec alarm to break the wait just in case the
		 * child doesn't terminate on its own accord
		 */
		(void)alarm(TIME_OUT);

		waitpid(child, &status, 0);
		if (WEXITSTATUS(status) != 0) {
			fail = 1;
			tst_resm(TFAIL, "child returned bad exit status");
		}

		/* turn off the alarm clock */
		(void)alarm((unsigned)0);
		if (status != 0) {
			tst_resm(TFAIL, "child returned status 0x%x", status);
			fail = 1;
		}
		close(fd);
		unlink(tmpname);

		testcheck_end(fail, "Test of locks on file");
		tst_resm(TINFO, "Exit block 4");
	}
	cleanup();
	tst_exit();
}
