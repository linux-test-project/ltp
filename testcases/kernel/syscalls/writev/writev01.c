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
 *	writev01.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of writev(2) system call.
 *
 * ALGORITHM
 *	Create a IO vector, and attempt to writev() various components of it.
 *
 * USAGE:  <for command-line>
 *	writev01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,	-c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *      04/2002 wjhuie sigset cleanups
 *     06/2002 Shaobo Li
 *             fix testcase 7, add each testcase comment.
 *
 * Restrictions
 *	None
 */

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/uio.h>
#include <sys/fcntl.h>
#include <memory.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>
#include <sys/mman.h>

#define	K_1	1024
#define	M_1	K_1 * K_1
#define	G_1	M_1 * K_1

#define	NBUFS		4
#define	CHUNK		64	/* single chunk */
#define	MAX_IOVEC	16
#define	DATA_FILE	"writev_data_file"

char buf1[K_1], buf2[K_1], buf3[K_1];

struct iovec wr_iovec[MAX_IOVEC] = {
	/* iov_base *//* iov_len */

	/* testcase# 1 */
	{buf1, -1},
	{(buf1 + CHUNK), CHUNK},
	{(buf1 + CHUNK * 2), CHUNK},

	/* testcase# 2 */
	{(buf1 + CHUNK * 3), G_1},
	{(buf1 + CHUNK * 4), G_1},
	{(buf1 + CHUNK * 5), G_1},

	/* testcase# 3 */
	{(buf1 + CHUNK * 6), CHUNK},
	{(caddr_t) - 1, CHUNK},
	{(buf1 + CHUNK * 8), CHUNK},

	/* testcase# 4 */
	{(buf1 + CHUNK * 9), CHUNK},

	/* testcase# 5 */
	{(buf1 + CHUNK * 10), CHUNK},

	/* testcase# 6 */
	{(buf1 + CHUNK * 11), CHUNK},

	/* testcase# 7 */
	{(buf1 + CHUNK * 12), CHUNK},

	/* testcase# 8 */
	{(buf1 + CHUNK * 13), 0},

	/* testcase# 7 */
	{(caddr_t) NULL, 0},
	{(caddr_t) NULL, 0}
};

char name[K_1], f_name[K_1];

char *bad_addr = 0;

/* 0 terminated list of expected errnos */
int exp_enos[] = { 14, 22, 32, 77, 0 };

int fd[4], in_sighandler;
int pfd[2];			/* pipe fd's */
char *buf_list[NBUFS];
int fail;

void sighandler(int);
long l_seek(int, long, int);
int fill_mem(char *, int, int);
int init_buffs(char *[]);
void setup(void);
void cleanup(void);

char *TCID = "writev01";
int TST_TOTAL = 1;
extern int Tst_count;

int main(int argc, char **argv)
{
	int nbytes, ret;

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	/* set "tstdir", and "testfile" vars */
	setup();

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		buf_list[0] = buf1;
		buf_list[1] = buf2;
		buf_list[2] = buf3;
		buf_list[3] = (char *)NULL;

		fd[1] = -1;	/* Invalid file descriptor  */

		if (signal(SIGTERM, sighandler) == SIG_ERR) {
			perror("signal: SIGTERM");
			cleanup();
		 /*NOTREACHED*/}

		if (signal(SIGPIPE, sighandler) == SIG_ERR) {
			perror("signal: SIGPIPE");
			cleanup();
		 /*NOTREACHED*/}

		init_buffs(buf_list);

		if ((fd[0] = open(f_name, O_WRONLY | O_CREAT, 0666)) < 0) {
			tst_resm(TFAIL, "open failed: fname = %s, errno = %d",
				 f_name, errno);
			cleanup();
		 /*NOTREACHED*/} else
		    if ((nbytes = write(fd[0], buf_list[2], K_1)) != K_1) {
			tst_resm(TFAIL,
				 "write failed: nbytes = %d, " "errno = %d",
				 nbytes, errno);
			cleanup();
		 /*NOTREACHED*/}

		if (close(fd[0]) < 0) {
			tst_resm(TFAIL, "close failed: errno: %d", errno);
			cleanup();
		 /*NOTREACHED*/}

		if ((fd[0] = open(f_name, O_RDWR, 0666)) < 0) {
			tst_resm(TFAIL, "open failed: fname = %s, errno = %d",
				 f_name, errno);
			cleanup();
		 /*NOTREACHED*/}
//block1: /* given vector length -1, writev() return EINVAL. */
		tst_resm(TINFO, "Enter Block 1");
		fail = 0;

		TEST(writev(fd[0], wr_iovec, 1));
		if (TEST_RETURN < 0) {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO == EINVAL) {
				tst_resm(TINFO, "Received EINVAL as expected");
			} else {
				tst_resm(TFAIL, "Expected errno = EINVAL, "
					 "got %d", TEST_ERRNO);
				fail = 1;
			}
		} else {
			tst_resm(TFAIL, "writev() failed to fail");
			fail = 1;
		}
		if (fail) {
			tst_resm(TINFO, "block 1 FAILED");
		} else {
			tst_resm(TINFO, "block 1 PASSED");
		}
		tst_resm(TINFO, "Exit block 1");

//block2:
		/* This testcases doesn't look like what it intent to do
		 * 1. it is not using the wr_iovec initialized
		 * 2. read() and following message is not consistent
		 */
		tst_resm(TINFO, "Enter block 2");
		fail = 0;

		if (l_seek(fd[0], CHUNK * 6, 0) < 0) {
			TEST_ERROR_LOG(errno);
			tst_resm(TBROK, "block2: 1st lseek failed");
			fail = 1;
		}

		if ((ret = writev(fd[0], (wr_iovec + 6), 3)) == CHUNK) {
			if (l_seek(fd[0], CHUNK * 6, 0) < 0) {
				TEST_ERROR_LOG(errno);
				tst_resm(TFAIL, "block2: 2nd lseek failed");
				fail = 1;
			}
			if ((nbytes = read(fd[0], buf_list[0], CHUNK)) != CHUNK) {
				perror("read error");
				tst_resm(TFAIL, "expected nbytes = 1024, "
					 "got = %d", nbytes);
				fail = 1;
			} else if (memcmp((buf_list[0] + CHUNK * 6),
					  (buf_list[2] + CHUNK * 6),
					  CHUNK) != 0) {
				tst_resm(TFAIL, "Error: writev() over "
					 "wrote %s", f_name);
				fail = 1;
			}
		} else {
			tst_resm(TFAIL, "writev() failed unexpectedly");
			fail = 1;
		}
		if (fail) {
			tst_resm(TINFO, "block 2 FAILED");
		} else {
			tst_resm(TINFO, "block 2 PASSED");
		}
		tst_resm(TINFO, "Exit block 2");

//block3: /* given 1 bad vector buffer with good ones, writev() success */
		tst_resm(TINFO, "Enter block 3");
		fail = 0;

		if (lseek(fd[0], CHUNK * 6, 0) < 0) {
			TEST_ERROR_LOG(errno);
			tst_resm(TFAIL, "block3: 1st lseek failed");
			fail = 1;
		}
		if ((nbytes = writev(fd[0], (wr_iovec + 6), 3)) < 0) {
			TEST_ERROR_LOG(errno);
			if (errno == EFAULT) {
				tst_resm(TFAIL, "Got EFAULT");
				fail = 1;
			}
		}
		if (l_seek(fd[0], 0, 0) < 0) {
			TEST_ERROR_LOG(errno);
			tst_resm(TFAIL, "block3: 2nd lseek failed");
			fail = 1;
		}
		if ((nbytes = read(fd[0], buf_list[0], K_1)) != K_1) {
			perror("read error");
			tst_resm(TFAIL, "expected nbytes = 1024, got = %d",
				 nbytes);
			fail = 1;
		} else if (memcmp((buf_list[0] + CHUNK * 6),
				  (buf_list[2] + CHUNK * 6), CHUNK * 3) != 0) {
			tst_resm(TFAIL, "Error: writev() over wrote %s",
				 f_name);
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "block 3 FAILED");
		} else {
			tst_resm(TINFO, "block 3 PASSED");
		}
		tst_resm(TINFO, "Exit block 3");

//block4: /* given bad file discriptor, writev() return EBADF. */
		tst_resm(TINFO, "Enter block 4");
		fail = 0;

		TEST(writev(fd[1], (wr_iovec + 9), 1));
		if (TEST_RETURN < 0) {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO == EBADF) {
				tst_resm(TINFO, "Received EBADF as expected");
			} else {
				tst_resm(TFAIL, "expected errno = EBADF, "
					 "got %d", TEST_ERRNO);
				fail = 1;
			}
		} else {
			tst_resm(TFAIL, "Error: writev() returned a "
				 "positive value");
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "block 4 FAILED");
		} else {
			tst_resm(TINFO, "block 4 PASSED");
		}
		tst_resm(TINFO, "Exit block 4");

//block5: /* given invalid vector count, writev() return EINVAL */
		tst_resm(TINFO, "Enter block 5");
		fail = 0;

		TEST(writev(fd[0], (wr_iovec + 10), -1));
		if (TEST_RETURN < 0) {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO == EINVAL) {
				tst_resm(TINFO, "Received EINVAL as expected");
			} else {
				tst_resm(TFAIL, "expected errno = EINVAL, "
					 "got %d", TEST_ERRNO);
				fail = 1;
			}
		} else {
			tst_resm(TFAIL, "Error: writev() returned a "
				 "positive value");
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "block 5 FAILED");
		} else {
			tst_resm(TINFO, "block 5 PASSED");
		}
		tst_resm(TINFO, "Exit block 5");

//block6: /* given no buffer vector, writev() success */
		tst_resm(TINFO, "Enter block 6");
		fail = 0;

		TEST(writev(fd[0], (wr_iovec + 11), 0));
		if (TEST_RETURN < 0) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "writev() failed with unexpected errno "
				 "%d", TEST_ERRNO);
			fail = 1;
		} else {
			tst_resm(TPASS, "writev() wrote 0 iovectors");
		}

		if (fail) {
			tst_resm(TINFO, "block 6 FAILED");
		} else {
			tst_resm(TINFO, "block 6 PASSED");
		}
		tst_resm(TINFO, "Exit block 6");

//block7:
		/* given 4 vectors, 2 are NULL, 1 with 0 length and 1 with fixed length,
		 * writev() success writing fixed length.
		 */
		tst_resm(TINFO, "Enter block 7");
		fail = 0;

		l_seek(fd[0], CHUNK * 12, 0);
		if ((ret = writev(fd[0], (wr_iovec + 12), 4)) != CHUNK) {
			tst_resm(TFAIL, "writev() failed writing %d bytes, "
				 "followed by two NULL vectors", CHUNK);
			fail = 1;
		} else {
			tst_resm(TPASS, "writev passed writing %d bytes, "
				 "followed by two NULL vectors", CHUNK);
		}

		if (fail) {
			tst_resm(TINFO, "block 7 FAILED");
		} else {
			tst_resm(TINFO, "block 7 PASSED");
		}
		tst_resm(TINFO, "Exit block 7");

//block8: /* try to write to a closed pipe, writev() return EPIPE. */
		tst_resm(TINFO, "Enter block 8");
		fail = 0;

		if (pipe(pfd) < 0) {
			TEST_ERROR_LOG(errno);
			perror("pipe");
			tst_resm(TFAIL, "pipe failed: errno = %d", errno);
			fail = 1;
		} else {
			if (close(pfd[0]) < 0) {
				TEST_ERROR_LOG(errno);
				perror("close");
				tst_resm(TFAIL, "close failed: errno = %d",
					 errno);
				fail = 1;
			} else if ((writev(pfd[1], (wr_iovec + 12), 1)
				    < 0) && in_sighandler) {
				TEST_ERROR_LOG(errno);
				if (errno == EPIPE) {
					tst_resm(TINFO, "Received EPIPE as "
						 "expected");
				} else {
					tst_resm(TFAIL, "expected errno = "
						 "EPIPE, got %d", errno);
					fail = 1;
				}
			} else {
				tst_resm(TFAIL, "Error: writev() returned a "
					 "positive value");
				fail = 1;
			}
		}
		if (fail) {
			tst_resm(TINFO, "block 8 FAILED");
		} else {
			tst_resm(TINFO, "block 8 PASSED");
		}
		tst_resm(TINFO, "Exit block 8");
	}
	close(fd[0]);
	close(fd[1]);
	cleanup();
	 /*NOTREACHED*/ return 0;
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified.
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* Create a unique temporary directory and chdir() to it. */
	tst_tmpdir();

	strcpy(name, DATA_FILE);
	sprintf(f_name, "%s.%d", name, getpid());

	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		printf("mmap failed\n");
	}
	wr_iovec[7].iov_base = bad_addr;

}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	if (unlink(f_name) < 0) {
		tst_resm(TFAIL, "unlink Failed--file = %s, errno = %d",
			 f_name, errno);
	}
	tst_rmdir();

	tst_exit();
}

int init_buffs(char *pbufs[])
{
	int i;

	for (i = 0; pbufs[i] != (char *)NULL; i++) {
		switch (i) {
		case 0:

		case 1:
			fill_mem(pbufs[i], 0, 1);
			break;

		case 2:
			fill_mem(pbufs[i], 1, 0);
			break;

		default:
			tst_resm(TFAIL, "Error detected: init_buffs()");
			cleanup();
		 /*NOTREACHED*/}
	}
	return 0;
}

int fill_mem(char *c_ptr, int c1, int c2)
{
	int count;

	for (count = 1; count <= K_1 / CHUNK; count++) {
		if (count & 0x01) {	/* if odd */
			memset(c_ptr, c1, CHUNK);
		} else {	/* if even */
			memset(c_ptr, c2, CHUNK);
		}
	}
	return 0;
}

void sighandler(int sig)
{
	switch (sig) {
	case SIGTERM:
		break;

	case SIGPIPE:
		++in_sighandler;
		return;

	default:
		tst_resm(TFAIL, "sighandler() received invalid "
			 "signal:%d", sig);
		break;
	}

	if (unlink(f_name) < 0) {
		tst_resm(TFAIL, "unlink Failed--file = %s, errno = %d",
			 f_name, errno);
		tst_exit();
	 /*NOTREACHED*/}
	exit(sig);
}

long l_seek(int fdesc, long offset, int whence)
{
	if (lseek(fdesc, offset, whence) < 0) {
		perror("lseek");
		tst_resm(TFAIL, "lseek Failed : errno = %d", errno);
		fail = 1;
	}
	return 0;
}
