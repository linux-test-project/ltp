/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 John George
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 * Check the return value, and errno of vhangup(2) when a non-root user calls
 * vhangup().
 */
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/wait.h>

#include "test.h"
#include "safe_macros.h"

static void setup(void);

char *TCID = "vhangup01";
int TST_TOTAL = 1;

static uid_t nobody_uid;

int main(int argc, char **argv)
{
	int lc;

	pid_t pid;
	int retval;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TFAIL, NULL, "fork failed");
		} else if (pid > 0) {
			tst_record_childstatus(NULL, pid);
		} else {
			retval = setreuid(nobody_uid, nobody_uid);
			if (retval < 0) {
				perror("setreuid");
				tst_brkm(TFAIL, NULL, "setreuid failed");
			}
			TEST(vhangup());
			if (TEST_RETURN != -1) {
				tst_brkm(TFAIL, NULL, "vhangup() failed to "
					 "fail");
			} else if (TEST_ERRNO == EPERM) {
				tst_resm(TPASS, "Got EPERM as expected.");
			} else {
				tst_resm(TFAIL, "expected EPERM got %d",
					 TEST_ERRNO);
			}
		}
	}

	tst_exit();
}

static void setup(void)
{
	struct passwd *pw;

	tst_require_root();

	pw = SAFE_GETPWNAM(NULL, "nobody");
	nobody_uid = pw->pw_uid;

	TEST_PAUSE;
}
