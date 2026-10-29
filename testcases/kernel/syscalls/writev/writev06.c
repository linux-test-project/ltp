/*
 * Copyright (C) 2007  Monakhov Dmitriy
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	writev06.c
 *
 * DESCRIPTION
 *	These testcases are written to test fault in pages readable
 *	feature in generic write path. Because before actual write
 *	kernel may want check is buffer passed is realy readable.
 *
 * USAGE:  <for command-line>
 *      writev06 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -e   : Turn on errno logging.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *
 * Restrictions
 *	NONE
 */

#include <sys/types.h>
#include <signal.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include "test.h"
#include <sys/mman.h>

#define	K_1	1024
#define	NBUFS		2
#define	MAX_IOVEC	2
#define	DATA_FILE	"writev_data_file"

int page_size;

char *good_addr[2] = { NULL, NULL };
char *bad_addr[2] = { NULL, NULL };

struct iovec wr_iovec[MAX_IOVEC] = {
	{(caddr_t) - 1, 1},
	{(caddr_t) - 1, 1}
};

char name[K_1], f_name[K_1];
int fd[2], in_sighandler;

char *TCID = "writev06";
int TST_TOTAL = 1;

void sighandler(int);
void setup(void);
void cleanup(void);

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();		/* set "tstdir", and "testfile" vars */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		fd[1] = -1;	/* Invalid file descriptor */

		if (signal(SIGTERM, sighandler) == SIG_ERR) {
			perror("signal");
			tst_resm(TFAIL, "signal() SIGTERM FAILED");
			cleanup();

		}

		if (signal(SIGPIPE, sighandler) == SIG_ERR) {
			perror("signal");
			tst_resm(TFAIL, "signal() SIGPIPE FAILED");
			cleanup();

		}

		if ((fd[0] = open(f_name, O_WRONLY | O_CREAT, 0666)) < 0) {
			tst_resm(TFAIL, "open(2) failed: fname = %s, "
				 "errno = %d", f_name, errno);
			cleanup();

		}

		/*
		 * Iovecs passed to writev points to valid (readable) regions,
		 * so all bytes must be successfully written.
		 */
		TEST(writev(fd[0], wr_iovec, 2));
		if (TEST_RETURN >= 0) {
			if (TEST_RETURN == 2) {
				tst_resm(TPASS,
					 "writev returned %d as expected", 2);
			} else {
				tst_resm(TFAIL, "Expected nbytes = %d, got "
					 "%ld", 2, TEST_RETURN);
			}
		} else {
			tst_resm(TFAIL | TTERRNO,
				 "Error writev return value = %ld",
				 TEST_RETURN);
		}
	}
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

	page_size = getpagesize();

	/* Crate two readable and writeble mappings with non reabable
	 * mapping around */
	bad_addr[0] = mmap(NULL, page_size * 3, PROT_NONE,
			   MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (bad_addr[0] == MAP_FAILED)
		tst_brkm(TBROK, cleanup, "mmap failed for bad_addr[0]");

	good_addr[0] = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (good_addr[0] == MAP_FAILED)
		tst_brkm(TBROK, cleanup, "mmap failed for good_addr[0]");

	bad_addr[1] = mmap(NULL, page_size * 3, PROT_NONE,
			   MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (bad_addr[1] == MAP_FAILED)
		tst_brkm(TBROK, cleanup, "mmap failed for bad_addr[1]");

	good_addr[1] = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (good_addr[1] == MAP_FAILED)
		tst_brkm(TBROK, cleanup, "mmap failed for good_addr[1]");

	/* force page fault for writable mappings */
	memset(good_addr[0], 'a', page_size);
	memset(good_addr[1], 'b', page_size);

	wr_iovec[0].iov_base = good_addr[0] + page_size - 1;
	wr_iovec[1].iov_base = good_addr[1] + page_size - 1;

}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{

	close(fd[0]);

	if (unlink(f_name) < 0) {
		tst_resm(TFAIL, "unlink Failed--file = %s, errno = %d",
			 f_name, errno);
	}
	tst_rmdir();

}

/*
 * sighandler()
 *	Signal handler for SIGTERM and SIGPIPE
 */
void sighandler(int sig)
{
	switch (sig) {
	case SIGTERM:
		break;
	case SIGPIPE:
		++in_sighandler;
		return;
	default:
		tst_resm(TFAIL, "sighandler() received invalid signal "
			 ": %d", sig);
		break;
	}

	if ((unlink(f_name) < 0) && (errno != ENOENT)) {
		tst_resm(TFAIL, "unlink Failed--file = %s, errno = %d",
			 f_name, errno);
		cleanup();

	}
	exit(sig);
}
