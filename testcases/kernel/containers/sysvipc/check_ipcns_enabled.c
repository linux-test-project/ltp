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
* Author: Rishikesh K Rajak <risrajak@in.ibm.com>
***************************************************************************/
#include <sched.h>
#include <stdio.h>
#include "../libclone/libclone.h"
#include "test.h"

int dummy(void *v)
{
        return 0;
}
int main()
{
        int pid;

        if (tst_kvercmp(2,6,19) < 0)
                return 1;

	pid = ltp_clone_quick(CLONE_NEWIPC, dummy, NULL);

        if (pid == -1)
                return 3;
        return 0;
}

