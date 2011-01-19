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
 *	Create a IO vector, and attempt to writev various components of it.
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
#include "test.h"
#include "usctest.h"
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

void sighandler(int);
int fill_mem(char *, int, int);
void init_buffs(char *[]);
void setup(void);
void cleanup(void);

char *TCID = "writev01";
int TST_TOTAL = 1;

int main(int argc, char **argv)
{
	int nbytes, ret;

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		buf_list[0] = buf1;
		buf_list[1] = buf2;
		buf_list[2] = buf3;
		buf_list[3] = NULL;

		fd[1] = -1;	/* Invalid file descriptor  */

		if (signal(SIGTERM, sighandler) == SIG_ERR)
			tst_brkm(TBROK|TERRNO, cleanup,
			    "signal(SIGTERM, ..) failed");

		if (signal(SIGPIPE, sighandler) == SIG_ERR)
			tst_brkm(TBROK|TERRNO, cleanup,
			    "signal(SIGPIPE, ..) failed");

		init_buffs(buf_list);

		if ((fd[0] = open(f_name, O_WRONLY | O_CREAT, 0666)) == -1)
			tst_brkm(TBROK|TERRNO, cleanup,
			    "open(.., O_WRONLY|O_CREAT, ..) failed");
		else
			if ((nbytes = write(fd[0], buf_list[2], K_1)) != K_1)
				tst_brkm(TBROK|TERRNO, cleanup, "write failed");

		if (close(fd[0]) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "close failed");

		if ((fd[0] = open(f_name, O_RDWR, 0666)) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "open failed");
//block1: /* given vector length -1, writev return EINVAL. */
		tst_resm(TPASS, "Enter Block 1");

		TEST(writev(fd[0], wr_iovec, 1));
		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == EINVAL)
				tst_resm(TPASS, "Received EINVAL as expected");
			else
				tst_resm(TFAIL, "Expected errno = EINVAL, "
					 "got %d", TEST_ERRNO);
		} else
			tst_resm(TFAIL, "writev failed to fail");
		tst_resm(TINFO, "Exit block 1");

//block2:
		/* This testcases doesn't look like what it intent to do
		 * 1. it is not using the wr_iovec initialized
		 * 2. read() and following message is not consistent
		 */
		tst_resm(TPASS, "Enter block 2");

		if (lseek(fd[0], CHUNK * 6, 0) == -1)
			tst_resm(TBROK|TERRNO, "block2: 1st lseek failed");

		if ((ret = writev(fd[0], (wr_iovec + 6), 3)) == CHUNK) {
			if (lseek(fd[0], CHUNK * 6, 0) == -1)
				tst_brkm(TBROK|TERRNO, cleanup,
				    "block2: 2nd lseek failed");
			if ((nbytes = read(fd[0], buf_list[0], CHUNK)) !=
			    CHUNK)
				tst_resm(TFAIL, "read failed; expected nbytes "
				    "= 1024, got = %d", nbytes);
			else if (memcmp((buf_list[0] + CHUNK * 6),
					  (buf_list[2] + CHUNK * 6),
					  CHUNK) != 0)
				tst_resm(TFAIL, "writev over "
					 "wrote %s", f_name);
		} else
			tst_resm(TFAIL|TERRNO, "writev failed unexpectedly");
		tst_resm(TINFO, "Exit block 2");

//block3: /* given 1 bad vector buffer with good ones, writev success */
		tst_resm(TPASS, "Enter block 3");

		if (lseek(fd[0], CHUNK * 6, 0) == -1)
			tst_brkm(TBROK|TERRNO, cleanup,
			    "block3: 1st lseek failed");
		if ((nbytes = writev(fd[0], (wr_iovec + 6), 3)) == -1) {
			if (errno == EFAULT)
				tst_resm(TFAIL, "Got EFAULT");
		}
		if (lseek(fd[0], 0, 0) == -1)
			tst_brkm(TBROK, cleanup, "block3: 2nd lseek failed");
		if ((nbytes = read(fd[0], buf_list[0], K_1)) != K_1) {
			tst_resm(TFAIL|TERRNO,
			    "read failed; expected nbytes = 1024, got = %d",
			    nbytes);
		} else if (memcmp((buf_list[0] + CHUNK * 6),
				  (buf_list[2] + CHUNK * 6), CHUNK * 3) != 0)
			tst_resm(TFAIL, "writev overwrote file");

		tst_resm(TINFO, "Exit block 3");

//block4: /* given bad file discriptor, writev return EBADF. */
		tst_resm(TPASS, "Enter block 4");

		TEST(writev(fd[1], (wr_iovec + 9), 1));
		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == EBADF)
				tst_resm(TPASS, "Received EBADF as expected");
			else
				tst_resm(TFAIL, "expected errno = EBADF, "
					 "got %d", TEST_ERRNO);
		} else
			tst_resm(TFAIL, "writev returned a "
				 "positive value");

		tst_resm(TINFO, "Exit block 4");

//block5: /* given invalid vector count, writev return EINVAL */
		tst_resm(TPASS, "Enter block 5");

		TEST(writev(fd[0], (wr_iovec + 10), -1));
		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == EINVAL)
				tst_resm(TPASS, "Received EINVAL as expected");
			else
				tst_resm(TFAIL, "expected errno = EINVAL, "
					 "got %d", TEST_ERRNO);
		} else
			tst_resm(TFAIL, "writev returned a "
				 "positive value");

		tst_resm(TINFO, "Exit block 5");

//block6: /* given no buffer vector, writev success */
		tst_resm(TPASS, "Enter block 6");

		TEST(writev(fd[0], (wr_iovec + 11), 0));
		if (TEST_RETURN == -1)
			tst_resm(TFAIL|TTERRNO, "writev failed");
		else
			tst_resm(TPASS, "writev wrote 0 iovectors");

		tst_resm(TINFO, "Exit block 6");

//block7:
		/* given 4 vectors, 2 are NULL, 1 with 0 length and 1 with fixed length,
		 * writev success writing fixed length.
		 */
		tst_resm(TPASS, "Enter block 7");

		if (lseek(fd[0], CHUNK * 12, 0) == -1)
			tst_resm(TBROK, "lseek failed");
		else if ((ret = writev(fd[0], (wr_iovec + 12), 4)) != CHUNK)
			tst_resm(TFAIL, "writev failed writing %d bytes, "
				 "followed by two NULL vectors", CHUNK);
		else
			tst_resm(TPASS, "writev passed writing %d bytes, "
				 "followed by two NULL vectors", CHUNK);

		tst_resm(TINFO, "Exit block 7");

//block8: /* try to write to a closed pipe, writev return EPIPE. */
		tst_resm(TPASS, "Enter block 8");

		if (pipe(pfd) == -1)
			tst_resm(TFAIL|TERRNO, "pipe failed");
		else {
			if (close(pfd[0]) == -1)
				tst_resm(TFAIL|TERRNO, "close failed");
			else if (writev(pfd[1], (wr_iovec + 12), 1) == -1 &&
			    in_sighandler) {
				if (errno == EPIPE)
					tst_resm(TPASS, "Received EPIPE as "
						 "expected");
				else
					tst_resm(TFAIL|TERRNO,
					    "didn't get EPIPE");
			} else
				tst_resm(TFAIL, "writev returned a positive "
						"value");
		}
		tst_resm(TINFO, "Exit block 8");
	}
	cleanup();
	tst_exit();
}

void setup(void)
{

	tst_sig(FORK, sighandler, cleanup);

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	tst_tmpdir();

	strcpy(name, DATA_FILE);
	sprintf(f_name, "%s.%d", name, getpid());

	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED)
		tst_brkm(TBROK|TERRNO, cleanup, "mmap failed");
	wr_iovec[7].iov_base = bad_addr;

}

void cleanup(void)
{
	TEST_CLEANUP;

	if (munmap(bad_addr, 1) == -1)
		tst_resm(TBROK|TERRNO, "munmap failed");

	close(fd[0]);
	close(fd[1]);

	if (unlink(f_name) == -1)
		tst_resm(TBROK|TERRNO, "unlink failed");

	tst_rmdir();

}

void init_buffs(char *pbufs[])
{
	int i;

	for (i = 0; pbufs[i] != NULL; i++) {
		switch (i) {
		case 0:

		case 1:
			fill_mem(pbufs[i], 0, 1);
			break;

		case 2:
			fill_mem(pbufs[i], 1, 0);
			break;

		default:
			tst_brkm(TBROK, cleanup, "error detected: init_buffs");
		}
	}
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
		in_sighandler++;
		return;

	default:
		tst_resm(TFAIL, "sighandler received invalid signal:%d", sig);
		break;
	}
}