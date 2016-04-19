/*
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
 * Test Name: fcntl22
 *
 * Test Description:
 *  Verify that, fcntl() fails with -1 and sets errno to EAGAIN when
 *				  Operation  is  prohibited  by locks held by other processes.
 *
 * Expected Result:
 *  fcntl() should fail with return value -1 and sets expected errno.
 *
 * HISTORY
 *		 06/2002 Ported by Jacky Malcles
 */

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"

int child_pid;
int file;
struct flock fl;

char *TCID = "fcntl22";
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		child_pid = FORK_OR_VFORK();
		switch (child_pid) {
		case 0:
			TEST(fcntl(file, F_SETLK, &fl));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "fcntl() returned %ld,"
					 "expected -1, errno=%d", TEST_RETURN,
					 EAGAIN);
			} else {
				if (TEST_ERRNO == EAGAIN) {
					tst_resm(TPASS,
						 "fcntl() fails with expected "
						 "error EAGAIN errno:%d",
						 TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "fcntl() fails, EAGAIN, "
						 "errno=%d, expected errno=%d",
						 TEST_ERRNO, EAGAIN);
				}
			}
			tst_exit();
		break;
		case -1:
			tst_brkm(TBROK|TERRNO, cleanup, "Fork failed");
		break;
		default:
			tst_record_childstatus(cleanup, child_pid);
		}

	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if ((file = creat("regfile", 0777)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "creat(regfile, 0777) failed, errno:%d %s", errno,
			 strerror(errno));
	}

	fl.l_type = F_WRLCK;
	fl.l_whence = 0;
	fl.l_start = 0;
	fl.l_len = 0;

	if (fcntl(file, F_SETLK, &fl) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fcntl() failed");
}

void cleanup(void)
{
	close(file);

	tst_rmdir();
}
