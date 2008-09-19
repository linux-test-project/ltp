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
#include "../libclone/libclone.h"
#include "test.h"

int main()
{
    int ret;
    long flags = 0;

    flags |= CLONE_NEWNS;
    flags |= CLONE_NEWNET;


	// Checking if the kernel ver is enough to do NET-NS testing.
	if (tst_kvercmp(2,6,24) < 0)
		return 1;

        ret = unshare(flags);
	if ( ret < 0 ) {
		printf ("Error:Unshare syscall failed for network namespace\n");
		return 3;
	}
	return 0;
}
