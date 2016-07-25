/* Copyright (c) 2014 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 2 the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 * File: mountns03.c
 *
 * Tests a slave mount: slave mount is like a shared mount except that
 * mount and umount events only propagate towards it.
 *
 * Description:
 * 1. Creates directories "A", "B" and files "A/A", "B/B"
 * 2. Unshares mount namespace and makes it private (so mounts/umounts
 *    have no effect on a real system)
 * 3. Bind mounts directory "A" to itself
 * 4. Makes directory "A" shared
 * 5. Clones a new child process with CLONE_NEWNS flag and makes "A"
 *    a slave mount
 * 6. There are two testcases (where X is parent namespace and Y child
 *    namespace):
 *    1)
 *	X: bind mounts "B" to "A"
 *	Y: must see the file "A/B"
 *	X: umounts "A"
 *    2)
 *	Y: bind mounts "B" to "A"
 *	X: must see only the "A/A" and must not see "A/B" (as slave
 *	   mount does not forward propagation)
 *	Y: umounts "A"
 ***********************************************************************/

#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "mountns_helper.h"
#include "test.h"
#include "safe_macros.h"

char *TCID	= "mountns03";
int TST_TOTAL	= 2;

#if defined(MS_SHARED) && defined(MS_PRIVATE) \
    && defined(MS_REC) && defined(MS_SLAVE)

int child_func(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret = 0;

	/* makes mount DIRA a slave of DIRA (all slave mounts have
	 * a master mount which is a shared mount) */
	if (mount("none", DIRA, "none", MS_SLAVE, NULL) == -1) {
		perror("mount");
		return 1;
	}

	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	/* checks that shared mounts propagates to slave mount */
	if (access(DIRA"/B", F_OK) == -1)
		ret = 2;

	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	/* bind mounts DIRB to DIRA making contents of DIRB visible
	 * in DIRA */
	if (mount(DIRB, DIRA, "none", MS_BIND, NULL) == -1) {
		perror("mount");
		return 1;
	}

	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	umount(DIRA);
	return ret;
}

static void test(void)
{
	int status;

	/* unshares the mount ns */
	if (unshare(CLONE_NEWNS) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "unshare failed");
	/* makes sure parent mounts/umounts have no effect on a real system */
	SAFE_MOUNT(cleanup, "none", "/", "none", MS_REC|MS_PRIVATE, NULL);

	/* bind mounts DIRA to itself */
	SAFE_MOUNT(cleanup, DIRA, DIRA, "none", MS_BIND, NULL);

	/* makes mount DIRA shared */
	SAFE_MOUNT(cleanup, "none", DIRA, "none", MS_SHARED, NULL);

	if (do_clone_tests(CLONE_NEWNS, child_func, NULL, NULL, NULL) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");

	/* waits for child to make a slave mount */
	TST_SAFE_CHECKPOINT_WAIT(cleanup, 0);

	/* bind mounts DIRB to DIRA making contents of DIRB visible
	 * in DIRA */
	SAFE_MOUNT(cleanup, DIRB, DIRA, "none", MS_BIND, NULL);

	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(cleanup, 0);

	SAFE_UMOUNT(cleanup, DIRA);

	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(cleanup, 0);

	/* checks that slave mount doesn't propagate to shared mount */
	if ((access(DIRA"/A", F_OK) == 0) && (access(DIRA"/B", F_OK) == -1))
		tst_resm(TPASS, "propagation from slave mount passed");
	else
		tst_resm(TFAIL, "propagation form slave mount failed");

	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);


	SAFE_WAIT(cleanup, &status);
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 0)
			tst_resm(TPASS, "propagation to slave mount passed");
		else
			tst_resm(TFAIL, "propagation to slave mount failed");
	}
	if (WIFSIGNALED(status)) {
		tst_resm(TBROK, "child was killed with signal %s",
			 tst_strsig(WTERMSIG(status)));
		return;
	}

	SAFE_UMOUNT(cleanup, DIRA);
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++)
		test();

	cleanup();
	tst_exit();
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, "needed mountflags are not defined");
}
#endif
