/*
 *
 *   Copyright (C) Bull S.A. 2001
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
 * 	readv03.c
 *
 * DESCRIPTION
 *	Testcase to check the error condition of the readv(2) system call
 *	when fd refers to a directory.
 *
 * CALLS
 * 	readv()
 *
 * ALGORITHM
 *      loop if that option was specified
 *      call readv() when fd refers to a directory.
 *      check the errno value
 *        issue a PASS message if we get EISDIR - errno 21
 *      otherwise, the tests fails
 *        issue a FAIL message
 *      call cleanup
 *
 * USAGE$
 *	readv03
 *
 * HISTORY
 *	05/2002 Ported by Jacky Malcles
 *
 * RESTRICTIONS
 * 	None
 */
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "test.h"
#include "safe_macros.h"

#define	K_1	1024
#define MODES   S_IRWXU

char buf1[K_1];

struct iovec rd_iovec[1] = {
	{buf1, K_1}
};

const char *TEST_DIR = "alpha";
int r_val;
int fd;

char *TCID = "readv03";
int TST_TOTAL = 1;

void setup();
void cleanup();

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		if (readv(fd, rd_iovec, 1) < 0) {
			if (errno != EISDIR) {
				tst_resm(TFAIL, "expected errno = EISDIR, "
					 "got %d", errno);
			} else {
				tst_resm(TPASS, "got EISDIR");
			}
		} else {
			tst_resm(TFAIL, "Error: readv returned a positive "
				 "value");
		}

	}
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	/*
	 * create a new directory and open it
	 */

	if ((r_val = mkdir(TEST_DIR, MODES)) == -1) {
		tst_brkm(TBROK, cleanup, "%s - mkdir() in main() "
			 "failed", TCID);
	}

	if ((fd = open(TEST_DIR, O_RDONLY)) == -1) {
		tst_brkm(TBROK, cleanup, "open of directory failed");
	}

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{
	SAFE_CLOSE(cleanup, fd);
	tst_rmdir();

}
