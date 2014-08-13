/*
* Copyright (c) International Business Machines Corp., 2007
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
* Author: Rishikesh K Rajak <risrajak@in.ibm.com>
***************************************************************************/
#include <sched.h>
#include "../libclone/libclone.h"
#include "test.h"
#include "safe_macros.h"

static int dummy_child(void *v)
{
	(void) v;
	return 0;
}

static void check_newipc(void)
{
	int pid, status;

	if (tst_kvercmp(2, 6, 19) < 0)
		tst_brkm(TCONF, NULL, "CLONE_NEWIPC not supported");

	pid = do_clone_unshare_test(T_CLONE, CLONE_NEWIPC, dummy_child, NULL);
	if (pid == -1)
		tst_brkm(TCONF | TERRNO, NULL, "CLONE_NEWIPC not supported");

	SAFE_WAIT(NULL, &status);
}
