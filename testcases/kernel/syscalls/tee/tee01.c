/****************
 *
 *   Copyright (c) International Business Machines  Corp., 2006
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
 *
 * NAME
 *      tee01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of tee(2)
 *	added by kernel 2.6.17 or up.
 *
 * USAGE:  <for command-line>
 * tee01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
 * where:
 *      -c n : Run n copies simultaneously.
 *      -e   : Turn on errno logging.
 *      -i n : Execute test n times.
 *      -I x : Execute test for x seconds.
 *      -p   : Pause for SIGUSR1 before starting
 *      -P x : Pause for x seconds between iterations.
 *      -t   : Turn on syscall timing.
 *
 * Author
 *	Yi Yang <yyangcdl@cn.ibm.com>
 *
 * History
 *      08/18/2006      Created first by Yi Yang <yyangcdl@cn.ibm.com>
 *
 ***************/

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

#define SPLICE_TEST_BLOCK_SIZE 1024
#define SPLICE_F_NONBLOCK (0x02)

static int tee_test(void);
void setup();
void cleanup();

char *TCID = "tee01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
char testfile1[256];
char testfile2[256];

static inline long splice(int fd_in, loff_t * off_in,
			  int fd_out, loff_t * off_out,
			  size_t len, unsigned int flags)
{
	return syscall(__NR_splice, fd_in, off_in, fd_out, off_out, len, flags);
}

static inline int tee(int fdin, int fdout, size_t len, unsigned int flags)
{
	return syscall(__NR_tee, fdin, fdout, len, flags);
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int results;

	/* Disable test if the version of the kernel is less than 2.6.17 */
	if (((results = tst_kvercmp(2, 6, 17)) < 0)) {
		tst_resm(TWARN, "This test can only run on kernels that are ");
		tst_resm(TWARN, "2.6.17 and higher");
		exit(0);
	}

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check if the current filesystem is nfs
	 */
	if (tst_is_cwd_nfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do tee on a file located on an NFS filesystem");
	}

	/*
	 * check looping state if -c option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call tee_test
		 */
		TEST(tee_test());

		/* check return code */
		if (TEST_RETURN < 0) {
			if (TEST_RETURN != -1) {
				TEST_ERRNO = -TEST_RETURN;
			}
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "tee() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {

			/*
			 * only perform functional verification if flag set (-f not given)
			 */
			if (STD_FUNCTIONAL_TEST) {
				/* No Verification test, yet... */
				tst_resm(TPASS, "tee() returned %ld",
					 TEST_RETURN);
			}
		}

	}

	/*
	 * cleanup and exit
	 */
	cleanup();

	return (0);
}

static int tee_test(void)
{
	char buffer[SPLICE_TEST_BLOCK_SIZE];
	char teebuffer[SPLICE_TEST_BLOCK_SIZE];
	int pipe1[2];
	int pipe2[2];
	int ret;
	int i, len;
	int fd_in, fd_out;

	for (i = 0; i < SPLICE_TEST_BLOCK_SIZE; i++) {
		buffer[i] = i & 0xff;
	}

	fd_in = open(testfile1, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd_in < 0) {
		perror("open: ");
		return -1;
	}
	len = write(fd_in, buffer, SPLICE_TEST_BLOCK_SIZE);
	if (len < SPLICE_TEST_BLOCK_SIZE) {
		perror("write: ");
		close(fd_in);
		return -1;
	}
	close(fd_in);

	fd_in = open(testfile1, O_RDONLY);
	if (fd_in < 0) {
		perror("open: ");
		return -1;
	}

	ret = pipe(pipe1);
	if (ret < 0) {
		perror("pipe: ");
		close(fd_in);
		return -1;
	}

	ret = pipe(pipe2);
	if (ret < 0) {
		perror("pipe: ");
		close(pipe1[0]);
		close(pipe1[1]);
		close(fd_in);
		return -1;
	}

	fd_out = open(testfile2, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd_out < 0) {
		close(fd_in);
		close(pipe1[0]);
		close(pipe1[1]);
		close(pipe2[0]);
		close(pipe2[1]);
		perror("open: ");
		return -1;
	}
	ret = splice(fd_in, NULL, pipe1[1], NULL, SPLICE_TEST_BLOCK_SIZE, 0);
	if (ret < 0) {
		ret = -errno;
		close(fd_in);
		close(fd_out);
		close(pipe1[0]);
		close(pipe1[1]);
		close(pipe2[0]);
		close(pipe2[1]);
		return ret;
	}

	ret =
	    tee(pipe1[0], pipe2[1], SPLICE_TEST_BLOCK_SIZE, SPLICE_F_NONBLOCK);
	if (ret < 0) {
		ret = -errno;
		close(fd_in);
		close(fd_out);
		close(pipe1[0]);
		close(pipe1[1]);
		close(pipe2[0]);
		close(pipe2[1]);
		return ret;
	}

	ret = splice(pipe2[0], NULL, fd_out, NULL, SPLICE_TEST_BLOCK_SIZE, 0);
	if (ret < 0) {
		ret = -errno;
		close(fd_in);
		close(fd_out);
		close(pipe1[0]);
		close(pipe1[1]);
		close(pipe2[0]);
		close(pipe2[1]);
		return ret;
	}

	close(fd_in);
	close(fd_out);
	close(pipe1[0]);
	close(pipe1[1]);
	close(pipe2[0]);
	close(pipe2[1]);

	fd_out = open(testfile2, O_RDONLY);
	if (fd_out < 0) {
		perror("open: ");
		return -1;
	}
	read(fd_out, teebuffer, SPLICE_TEST_BLOCK_SIZE);
	for (i = 0; i < SPLICE_TEST_BLOCK_SIZE; i++) {
		if (buffer[i] != teebuffer[i])
			break;
	}
	if (i < SPLICE_TEST_BLOCK_SIZE) {
		return -1;
	}
	return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_tmpdir();

	/* Initialize test file names */
	sprintf(testfile1, "teetest%d_1.txt", getpid());
	sprintf(testfile2, "teetest%d_2.txt", getpid());

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/* Remove them */
	unlink(testfile1);
	unlink(testfile2);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();

}