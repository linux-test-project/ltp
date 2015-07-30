/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 *  07/2001 Ported by Wayne Boyer
 *  21/04/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
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
 * Attempt to execve(2) an executable owned by root with no execute permissions
 * for the other users, fails when execve(2) is used as a non-root user, the
 * errno should be EACCES.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <libgen.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "execve02";
int TST_TOTAL = 1;

#define TEST_APP "execve_child"
#define USER_NAME "nobody"

static void setup(void);
static void cleanup(void);

static uid_t nobody_uid;

static void do_child(void)
{
	char *argv[2] = {TEST_APP, NULL};

	SAFE_SETEUID(NULL, nobody_uid);

	TEST(execve(TEST_APP, argv, NULL));

	if (!TEST_RETURN)
		tst_brkm(TFAIL, NULL, "execve() passed unexpectedly");

	if (TEST_ERRNO != EACCES) {
		tst_brkm(TFAIL | TTERRNO, NULL,
		         "execve() failed unexpectedly");
	}

	tst_resm(TPASS | TTERRNO, "execve() failed expectedly");
	tst_exit();
}

int main(int ac, char **av)
{
	int lc;
	pid_t pid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		if ((pid = FORK_OR_VFORK()) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "fork failed");

		if (pid == 0)
			do_child();

		tst_record_childstatus(cleanup, pid);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	char path[PATH_MAX];
	struct passwd *pwd;

	tst_require_root();

	if (tst_get_path(TEST_APP, path, sizeof(path))) {
		tst_brkm(TBROK, NULL,
		         "Couldn't found "TEST_APP" binary in $PATH");
	}

	tst_tmpdir();

	SAFE_CP(tst_rmdir, path, ".");
	SAFE_CHMOD(cleanup, TEST_APP, 0700);

	pwd = SAFE_GETPWNAM(tst_rmdir, USER_NAME);
	nobody_uid = pwd->pw_uid;
}

void cleanup(void)
{
	tst_rmdir();
}
