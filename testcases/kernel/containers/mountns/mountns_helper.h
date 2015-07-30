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
 */

#include "libclone.h"
#include "test.h"
#include "safe_macros.h"

#define DIRA "A"
#define DIRB "B"

static int dummy_child(void *v)
{
	(void) v;
	return 0;
}

static int check_newns(void)
{
	int pid, status;

	if (tst_kvercmp(2, 4, 19) < 0)
		tst_brkm(TCONF, NULL, "CLONE_NEWNS not supported");

	pid = do_clone_unshare_test(T_CLONE, CLONE_NEWNS, dummy_child, NULL);
	if (pid == -1)
		tst_brkm(TCONF | TERRNO, NULL, "CLONE_NEWNS not supported");
	SAFE_WAIT(NULL, &status);

	return 0;
}

static void cleanup(void)
{
	umount(DIRA);
	umount(DIRB);
	tst_rmdir();
}

static void setup(void)
{
	tst_require_root();
	check_newns();
	tst_tmpdir();
	TST_CHECKPOINT_INIT(tst_rmdir);
	SAFE_MKDIR(cleanup, DIRA, 0777);
	SAFE_MKDIR(cleanup, DIRB, 0777);
	SAFE_TOUCH(cleanup, DIRA"/A", 0, NULL);
	SAFE_TOUCH(cleanup, DIRB"/B", 0, NULL);
}
