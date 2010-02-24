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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
***************************************************************************

* File: check_pidns_enabled.c
*
* Description:
*  This testcase builds into the ltp framework to verify that kernel is
* PID NS enabled or not.
*
* Verify that:
* 1. Verify that the kernel version is 2.6.24.
* 2. Verify that clone() function return value.
*
* Test Name: check_pidns_enabled
*
* Test Assertion & Strategy:
* Check that the kernel version is 2.6.24 and clone returns no failure after passing
* the clone falg as CLONE_NEWPID.
*
* History:
*
* FLAG DATE     	NAME           		DESCRIPTION
* 27/12/07  RISHIKESH K RAJAK <risrajak@in.ibm.com> Created this test
*
*******************************************************************************************/
#include <sched.h>
#include <stdio.h>
#include "../libclone/libclone.h"
#include "test.h"

char *TCID = "check_pidns_enabled";
int TST_COUNT = 1;
int TST_TOTAL = 1;

int dummy(void *v)
{
	/* Simply return from the child */
        return 0;
}

/* MAIN */
int main()
{
        int pid;

        if (tst_kvercmp(2,6,24) < 0)
                return 1;

        pid = do_clone_unshare_test(T_CLONE, CLONE_NEWPID, dummy, NULL);

	/* Check for the clone function return value */
        if (pid == -1)
                return 3;
        return 0;
}

