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
*/

#include "../libclone/libclone.h"
#include "test.h"
#include "safe_macros.h"

static int dummy_child(void *v)
{
	(void) v;
	return 0;
}

static int check_newpid(void)
{
	int pid, status;

	if (tst_kvercmp(2, 6, 24) < 0)
		tst_brkm(TCONF, NULL, "CLONE_NEWPID not supported");

	pid = do_clone_unshare_test(T_CLONE, CLONE_NEWPID, dummy_child, NULL);
	if (pid == -1)
		tst_brkm(TCONF | TERRNO, NULL, "CLONE_NEWPID not supported");
	SAFE_WAIT(NULL, &status);

	return 0;
}
