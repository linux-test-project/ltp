/*
 * Copyright (c) International Business Machines Corp., 2009
 * Copyright (c) Nadia Derbey, 2009
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
 * Author: Serge Hallyn <serue@us.ibm.com>
 ***************************************************************************/
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include "../libclone/libclone.h"
#include "lapi/syscalls.h"
#include "safe_macros.h"
#include "test.h"

static int dummy_child(void *v)
{
	(void) v;
	return 0;
}

static void check_mqns(void)
{
	int pid, status;
	mqd_t mqd;

	if (tst_kvercmp(2, 6, 30) < 0)
		tst_brkm(TCONF, NULL, "Kernel version is lower than expected");

	mq_unlink("/checkmqnsenabled");
	mqd =
	    mq_open("/checkmqnsenabled", O_RDWR | O_CREAT | O_EXCL, 0777, NULL);
	if (mqd == -1)
		tst_brkm(TCONF, NULL, "mq_open check failed");

	mq_close(mqd);
	mq_unlink("/checkmqnsenabled");

	pid = do_clone_unshare_test(T_CLONE, CLONE_NEWIPC, dummy_child, NULL);
	if (pid == -1)
		tst_brkm(TCONF | TERRNO, NULL, "CLONE_NEWIPC not supported");

	SAFE_WAIT(NULL, &status);
}
