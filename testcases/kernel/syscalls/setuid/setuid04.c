/******************************************************************************
 * Copyright (c) Kerlabs 2008.                                                *
 * Copyright (c) International Business Machines  Corp., 2008                 *
 *  Created by Renaud Lottiaux                                                *
 *                                                                            *
 * This program is free software;  you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation; either version 2 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  *
 * the GNU General Public License for more details.                           *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program;  if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           *
 *****************************************************************************/

/*
 * Check if setuid behaves correctly with file permissions. The test creates a
 * file as ROOT with permissions 0644, does a setuid and then tries to open the
 * file with RDWR permissions. The same test is done in a fork to check if new
 * UIDs are correctly passed to the son.
 */

#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#include "test.h"
#include "compat_16.h"

char *TCID = "setuid04";
int TST_TOTAL = 1;

static char nobody_uid[] = "nobody";
static char testfile[] = "setuid04_testfile";
static struct passwd *ltpuser;

static int fd = -1;

static void setup(void);
static void cleanup(void);
static void do_master_child(void);

int main(int ac, char **av)
{
	pid_t pid;
	int status;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	pid = FORK_OR_VFORK();
	if (pid < 0)
		tst_brkm(TBROK, cleanup, "Fork failed");

	if (pid == 0) {
		do_master_child();
	} else {
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0))
			tst_resm(WEXITSTATUS(status),
				 "son process exits with error");
	}

	cleanup();
	tst_exit();
}

static void do_master_child(void)
{
	int lc;
	int pid;
	int status;

	if (SETUID(NULL, ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK, NULL,
			 "setuid failed to set the effective uid to %d",
			 ltpuser->pw_uid);
	}

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int tst_fd;

		tst_count = 0;

		TEST(tst_fd = open(testfile, O_RDWR));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			close(tst_fd);
		}

		if (TEST_ERRNO == EACCES) {
			tst_resm(TPASS, "open returned errno EACCES");
		} else {
			tst_resm(TFAIL, "open returned unexpected errno - %d",
				 TEST_ERRNO);
			continue;
		}

		pid = FORK_OR_VFORK();
		if (pid < 0)
			tst_brkm(TBROK, NULL, "Fork failed");

		if (pid == 0) {
			int tst_fd2;

			/* Test to open the file in son process */
			TEST(tst_fd2 = open(testfile, O_RDWR));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				close(tst_fd2);
			}

			if (TEST_ERRNO == EACCES) {
				tst_resm(TPASS, "open returned errno EACCES");
			} else {
				tst_resm(TFAIL,
					 "open returned unexpected errno - %d",
					 TEST_ERRNO);
			}
			tst_exit();
		} else {
			/* Wait for son completion */
			waitpid(pid, &status, 0);
			if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0))
				exit(WEXITSTATUS(status));
		}
	}
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	ltpuser = getpwnam(nobody_uid);

	if (ltpuser == NULL)
		tst_brkm(TBROK, cleanup, "getpwnam failed for user id %s",
			nobody_uid);

	UID16_CHECK(ltpuser->pw_uid, setuid, cleanup);

	tst_tmpdir();

	/* Create test file */
	fd = open(testfile, O_CREAT | O_RDWR, 0644);
	if (fd < 0)
		tst_brkm(TBROK, cleanup, "cannot creat test file");

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void cleanup(void)
{
	close(fd);
	tst_rmdir();
}
