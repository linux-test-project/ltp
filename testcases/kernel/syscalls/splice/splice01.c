/*
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
 *      splice01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of splice
 *	added by kernel 2.6.17 or up.
 *
 * USAGE:  <for command-line>
 * splice01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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
 */

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

static int splice_test(void);
void setup();
void cleanup();

char *TCID = "splice01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
char testfile1[256];
char testfile2[256];

static inline long splice(int fd_in, loff_t * off_in,
			  int fd_out, loff_t * off_out,
			  size_t len, unsigned int flags)
{
	return syscall(__NR_splice, fd_in, off_in, fd_out, off_out, len, flags);
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int results;

	/* Disable test if the version of the kernel is less than 2.6.17 */
	if (((results = tst_kvercmp(2, 6, 17)) < 0)) {
		tst_resm(TINFO, "This test can only run on kernels that are ");
		tst_resm(TINFO, "2.6.17 and higher");
		exit(0);
	}

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)))
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check if the current filesystem of the test directory is nfs
	 */
	tst_tmpdir();
	if (tst_is_cwd_nfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do splice on a file located on an NFS filesystem");
	}
	tst_rmdir();

	/*
	 * check looping state if -c option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call splice_test
		 */
		TEST(splice_test());

		/* check return code */
		if (TEST_RETURN < 0) {
			if (TEST_RETURN != -1) {
				TEST_ERRNO = -TEST_RETURN;
			}
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "splice() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {

			/*
			 * only perform functional verification if flag set (-f not given)
			 */
			if (STD_FUNCTIONAL_TEST) {
				/* No Verification test, yet... */
				tst_resm(TPASS, "splice() returned %ld",
					 TEST_RETURN);
			}
		}

	}			/* End for TEST_LOOPING */

	/*
	 * cleanup and exit
	 */
	cleanup();

	return (0);
}				/* End main */

static int splice_test(void)
{
	char buffer[SPLICE_TEST_BLOCK_SIZE];
	char splicebuffer[SPLICE_TEST_BLOCK_SIZE];
	int pipes[2];
	int ret;
	int i, len;
	int fd_in, fd_out;

	/* Make a temp directory and cd to it */
	tst_tmpdir();

	for (i = 0; i < SPLICE_TEST_BLOCK_SIZE; i++) {
		buffer[i] = i & 0xff;
	}

	fd_in = open(testfile1, O_WRONLY | O_CREAT | O_TRUNC, 0666);
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

	ret = pipe(pipes);
	if (ret < 0) {
		perror("pipe: ");
		close(fd_in);
		return -1;
	}

	fd_out = open(testfile2, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd_out < 0) {
		close(fd_in);
		close(pipes[0]);
		close(pipes[1]);
		perror("open: ");
		return -1;
	}
	ret = splice(fd_in, NULL, pipes[1], NULL, SPLICE_TEST_BLOCK_SIZE, 0);
	if (ret < 0) {
		ret = -errno;
		close(fd_in);
		close(fd_out);
		close(pipes[0]);
		close(pipes[1]);
		return ret;
	}
	splice(pipes[0], NULL, fd_out, NULL, SPLICE_TEST_BLOCK_SIZE, 0);
	if (ret < 0) {
		ret = -errno;
		close(fd_in);
		close(fd_out);
		close(pipes[0]);
		close(pipes[1]);
		return ret;
	}

	close(fd_in);
	close(fd_out);
	close(pipes[0]);
	close(pipes[1]);

	fd_out = open(testfile2, O_RDONLY);
	if (fd_out < 0) {
		perror("open: ");
		return -1;
	}
	read(fd_out, splicebuffer, SPLICE_TEST_BLOCK_SIZE);
	for (i = 0; i < SPLICE_TEST_BLOCK_SIZE; i++) {
		if (buffer[i] != splicebuffer[i])
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
	/* Initialize test file names */
	sprintf(testfile1, "splicetest%d_1.txt", getpid());
	sprintf(testfile2, "splicetest%d_2.txt", getpid());

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}				/* End setup() */

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

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
