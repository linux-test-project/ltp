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
 * File: mountns04.c
 *
 * Tests an unbindable mount: unbindable mount is an unbindable
 * private mount.
 * Description:
 * 1. Creates directories "A", "B" and files "A/A", "B/B"
 * 2. Unshares mount namespace and makes it private (so mounts/umounts
 *    have no effect on a real system)
 * 3. Bind mounts directory "A" to "A"
 * 4. Makes directory directory "A" unbindable
 * 5. Tries to bind mount unbindable "A" to "B":
 *    - if it fails, test passes
 *    - if it passes, test fails
 ***********************************************************************/

#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <errno.h>
#include "mountns_helper.h"
#include "test.h"
#include "safe_macros.h"

char *TCID	= "mountns04";
int TST_TOTAL	= 1;

#if defined(MS_SHARED) && defined(MS_PRIVATE) \
    && defined(MS_REC) && defined(MS_UNBINDABLE)

static void test(void)
{
	/* unshares the mount ns */
	if (unshare(CLONE_NEWNS) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "unshare failed");
	/* makes sure mounts/umounts have no effect on a real system */
	SAFE_MOUNT(cleanup, "none", "/", "none", MS_REC|MS_PRIVATE, NULL);

	/* bind mounts DIRA to itself */
	SAFE_MOUNT(cleanup, DIRA, DIRA, "none", MS_BIND, NULL);
	/* makes mount DIRA unbindable */
	SAFE_MOUNT(cleanup, "none", DIRA, "none", MS_UNBINDABLE, NULL);

	/* tries to bind mount unbindable DIRA to DIRB which should fail */
	if (mount(DIRA, DIRB, "none", MS_BIND, NULL) == -1) {
		tst_resm(TPASS, "unbindable mount passed");
	} else {
		SAFE_UMOUNT(cleanup, DIRB);
		tst_resm(TFAIL, "unbindable mount faled");
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
