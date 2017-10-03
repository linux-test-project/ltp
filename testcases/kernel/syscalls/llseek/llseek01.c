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
 * Test Name: llseek01
 *
 * Test Description:
 *  Verify that, llseek() call succeeds to set the file pointer position
 *  to an offset larger than file size. Also, verify that any attempt
 *  to write to this location fails.
 *
 * Expected result:
 *  llseek() should return the offset from the beginning of the file measured
 *  in bytes. Attempt to write to the location ( > file size) should fail.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  llseek01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  None.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>

#include "test.h"
#include "safe_macros.h"

#define TEMP_FILE	"tmp_file"
#define FILE_MODE	0644

char *TCID = "llseek01";
int TST_TOTAL = 1;
char write_buff[BUFSIZ];	/* buffer to hold data */
int fildes;			/* file handle for temp file */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;
	loff_t offset;		/* Ret value from llseek */

	tst_parse_opts(ac, av, NULL, NULL);

	offset = -1;

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * set file size limit, seek to a file using llseek.
		 */
		TEST(lseek64(fildes, (loff_t) (80 * BUFSIZ), SEEK_SET));

		if (TEST_RETURN == (loff_t) - 1) {
			tst_resm(TFAIL, "llseek on (%s) Failed, errno=%d : %s",
				 TEMP_FILE, TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		if (TEST_RETURN != (loff_t) (80 * BUFSIZ)) {
			tst_resm(TFAIL, "llseek() returned incorrect "
				 "value %" PRId64 ", expected %d",
				 (int64_t) offset, BUFSIZ);
			continue;
		}

		/*
		 * llseek() successful.  Now attempt to write past
		 * file size limit.
		 */
		if (write(fildes, write_buff, BUFSIZ) != -1) {
			tst_brkm(TFAIL, cleanup, "write successful "
				 "after file size limit");
		}

		/* Seeking to end of last valid write */
		offset = lseek64(fildes, (loff_t) BUFSIZ, SEEK_SET);
		if (offset != (loff_t) BUFSIZ) {
			tst_brkm(TFAIL, cleanup,
				 "llseek under file size limit");
		}

		/*
		 * llseek() successful.  Now, attempt to write to
		 * file size limit.
		 */
		if (write(fildes, write_buff, BUFSIZ) != BUFSIZ) {
			tst_brkm(TFAIL, cleanup, "write failed to "
				 "write to file size limit");
		}

		/*
		 * Again, attempt to write past file size limit.
		 */
		if (write(fildes, write_buff, BUFSIZ) != -1) {
			tst_brkm(TFAIL, cleanup, "write past file "
				 "size limit successful");
		}

		tst_resm(TPASS, "Functionality of llseek() on %s "
			 "successful", TEMP_FILE);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	struct sigaction act;	/* struct. to hold signal */
	struct rlimit rlp;	/* resource for file size limit */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGXFSZ, &act, NULL) == -1) {
		tst_brkm(TFAIL, NULL, "sigaction() Failed to ignore SIGXFSZ");
	}

	tst_tmpdir();

	/* Set limit low, argument is # bytes */
	rlp.rlim_cur = rlp.rlim_max = 2 * BUFSIZ;

	SAFE_SETRLIMIT(cleanup, RLIMIT_FSIZE, &rlp);

	/* Creat/open a temporary file under above directory */
	if ((fildes = open(TEMP_FILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0644) Failed, errno=%d :%s",
			 TEMP_FILE, errno, strerror(errno));
	}

	SAFE_WRITE(cleanup, 1, fildes, write_buff, BUFSIZ);
}

void cleanup(void)
{
	SAFE_CLOSE(NULL, fildes);

	tst_rmdir();
}
