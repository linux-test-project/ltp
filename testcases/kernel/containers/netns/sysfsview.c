
/*************************************************************************
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
***************************************************************************/

/* ============================================================================
* This testcase uses the libnetns.c from the lib to create network NS1.
* In libnetns.c it uses 2 scripts parentns.sh and childns.sh to create this.
*
* This testcase verifies sysfs contents of parentNS is visible from child NS.
* Also it checks the sysfs contents of the child are visible from the parent NS.
* On Success it returns PASS else returns FAIL
*
* Scripts used: parent_share.sh parent_view.sh child_propagate.sh
*               parentns.sh childns.sh
*
*
* Authors:      Poornima Nayak <poornima.nayak@in.ibm.com>
*               Veerendra C <vechandr@in.ibm.com>
*                      31/07/2008
* ============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "libclone.h"

int main()
{
    int ret, status = 0;
    char *script, *ltproot;

    ltproot = getenv("LTPROOT");
    if (! ltproot) {
        printf("LTPROOT env variable is not set\n");
        printf("Please set LTPROOT and re-run the test.. Thankyou\n");
        return -1;
    }

    script = malloc (FILENAME_MAX);
    if (script == NULL) {
        printf("FAIL: error while allocating mem");
        exit(1);
    }

    sprintf(script, "%s/testcases/bin/parent_share.sh" , ltproot);

    /* Parent should be able to view child sysfs and vice versa */
    ret = system(script);
    status = WEXITSTATUS(ret);
    if (ret == -1 || status != 0) {
        printf("Error while executing the script %s\n", script);
        fflush(stdout);
        exit(1);
    }

    status = create_net_namespace("parent_view.sh","child_propagate.sh");
    return status;
}