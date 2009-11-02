/******************************************************************************
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
 *      vmsplice01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of vmsplice
 *	added by kernel 2.6.17 or up.
 *
 * USAGE:  <for command-line>
 * vmsplice01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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
 *      09/01/2006      Created first by Yi Yang <yyangcdl@cn.ibm.com>
 *                      Thanks for some help from Jens Axboe <axboe@kernel.dk>
 *
 *****************************************************************************/

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/unistd.h>
#include <linux/uio.h>
#include <sys/poll.h>
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

#define SPLICE_TEST_BLOCK_SIZE (1<<17)	/* 128K */

static int vmsplice_test(void);
void setup();
void cleanup();

char *TCID = "vmsplice01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
char testfile[256];

static long mysplice(int fd_in, loff_t * off_in,
		     int fd_out, loff_t * off_out,
		     size_t len, unsigned int flags)
{
	return syscall(__NR_splice, fd_in, off_in, fd_out, off_out, len, flags);
}

static long myvmsplice(int fd, struct iovec *v, unsigned long nregs,
		       unsigned int flags)
{
	return syscall(__NR_vmsplice, fd, v, nregs, flags);
}

static void setup_every_copy()
{
	/* Initialize test file names */
	sprintf(testfile, "vmsplicetest%d.txt", getpid());
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Disable test if the version of the kernel is less than 2.6.17 */
	if ((tst_kvercmp(2, 6, 17)) < 0) {
		tst_resm(TWARN, "This test can only run on kernels that are ");
		tst_resm(TWARN, "2.6.17 and higher");
		exit(0);
	}

	/***************************************************************
	 * parse standard options
	 ***************************************************************/
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	/***************************************************************
	 * perform global setup for test
	 ***************************************************************/
	setup();

	/***************************************************************
	 * check looping state if -c option given
	 ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		setup_every_copy();

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call vmsplice_test
		 */
		TEST(vmsplice_test());

		/* check return code */
		if (TEST_RETURN < 0) {
			if (TEST_RETURN != -1) {
				TEST_ERRNO = -TEST_RETURN;
			}
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "vmsplice() Failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {

			/***************************************************************
			 * only perform functional verification if flag set (-f not given)
			 ***************************************************************/
			if (STD_FUNCTIONAL_TEST) {
				/* No Verification test, yet... */
				tst_resm(TPASS, "vmsplice() returned %ld",
					 TEST_RETURN);
			}
		}

	}			/* End for TEST_LOOPING */

	/***************************************************************
	 * cleanup and exit
	 ***************************************************************/
	cleanup();

	return (0);
}				/* End main */

static int vmsplice_test(void)
{
	char buffer[SPLICE_TEST_BLOCK_SIZE];
	char vmsplicebuffer[SPLICE_TEST_BLOCK_SIZE];
	int pipes[2];
	long written;
	int i, ret, flag = 0;
	int fd_out;
	struct iovec v;
	struct pollfd pfd;
	loff_t offset;

	for (i = 0; i < SPLICE_TEST_BLOCK_SIZE; i++) {
		buffer[i] = i & 0xff;
	}
	v.iov_base = buffer;
	v.iov_len = SPLICE_TEST_BLOCK_SIZE;

	ret = pipe(pipes);
	if (ret < 0) {
		perror("pipe: ");
		return -1;
	}

	fd_out = open(testfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd_out < 0) {
		close(pipes[0]);
		close(pipes[1]);
		perror("open: ");
		return -1;
	}

	pfd.fd = pipes[1];
	pfd.events = POLLOUT;
	offset = 0;
	while (v.iov_len) {
		/*
		 * in a real app you'd be more clever with poll of course,
		 * here we are basically just blocking on output room and
		 * not using the free time for anything interesting.
		 */
		if (poll(&pfd, 1, -1) < 0) {
			perror("poll: ");
			return -1;
		}

		written = myvmsplice(pipes[1], &v, 1, 0);
		if (written < 0) {
			ret = -errno;
			close(fd_out);
			close(pipes[0]);
			close(pipes[1]);
			return ret;
		} else if (written == 0)
			break;
		else {
			v.iov_base += written;
			v.iov_len -= written;
			flag = 1;
		}

		/*
		 * check if the current filesystem is nfs
		 */
		if (tst_is_cwd_nfs()) {
			if (flag == 1)
				tst_resm(TINFO, "vmplice() passes");
			tst_brkm(TCONF, cleanup, "Cannot do splice() "
				 "on a file located on an NFS filesystem");
		}

		ret = mysplice(pipes[0], NULL, fd_out, &offset, written, 0);
		if (ret < 0) {
			ret = -errno;
			close(fd_out);
			close(pipes[0]);
			close(pipes[1]);
			return ret;
		}
		//printf("offset = %lld\n", (long long)offset);

	}

	close(fd_out);
	close(pipes[0]);
	close(pipes[1]);

	fd_out = open(testfile, O_RDONLY);
	if (fd_out < 0) {
		perror("open: ");
		return -1;
	}
	read(fd_out, vmsplicebuffer, SPLICE_TEST_BLOCK_SIZE);
	for (i = 0; i < SPLICE_TEST_BLOCK_SIZE; i++) {
		if (buffer[i] != vmsplicebuffer[i])
			break;
	}
	if (i < SPLICE_TEST_BLOCK_SIZE) {
		return -1;
	}
	return 0;
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}				/* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/* Remove them */
	unlink(testfile);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
