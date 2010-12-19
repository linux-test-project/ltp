/*
* Copyright (c) International Business Machines Corp., 2008
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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* Author: Veerendra C <vechandr@in.ibm.com>
*
* Net namespaces were introduced around 2.6.25.  Kernels before that,
* assume they are not enabled.  Kernels after that, check for -EINVAL
* when trying to use CLONE_NEWNET and CLONE_NEWNS.
***************************************************************************/
#include <stdio.h>
#include <sched.h>
#include "config.h"
#include "libclone.h"
#include "linux_syscall_numbers.h"
#include "test.h"

char *TCID = "check_netns_enabled";
int TST_COUNT = 1;
int TST_TOTAL = 1;

#ifndef CLONE_NEWNET
#define CLONE_NEWNET -1
#endif

#ifndef CLONE_NEWNS
#define CLONE_NEWNS -1
#endif

int
main(void)
{
	/* Checking if the kernel supports unshare with netns capabilities. */
	if (CLONE_NEWNET == -1 || CLONE_NEWNS == -1)
		tst_resm(TBROK|TERRNO,
		    "CLONE_NEWNET (%d) or CLONE_NEWNS (%d) not supported",
		    CLONE_NEWNET, CLONE_NEWNS);
	else if (syscall(__NR_unshare, CLONE_NEWNET|CLONE_NEWNS) == -1)
		tst_resm(TFAIL|TERRNO, "unshare syscall smoke test failed");
	else
		tst_resm(TPASS, "unshare syscall smoke test passed");
	tst_exit();
}