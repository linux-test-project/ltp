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

/*******************************************************************************
* This testcase creates 2 network Namespace NS1 & NS2, oin the parent NS.
* It creates veth device pair for NS1 and NS2
* It checks the network connection between NS1 and NS2 .
* On Success returns PASS else returns FAIL.
*
* scripts used: parent_1.sh parent_2.sh child_1.sh child_2.sh
*
* Authors: Veerendra C <vechandr@in.ibm.com> ,
           Munipradeep <mbeeraka@in.ibm.com>
*                      31/07/2008
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <libgen.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <test.h>
#include "libclone.h"
#include "config.h"

char *TCID = "netns_2children";
int TST_TOTAL = 1;

int main()
{
    int ret, pid[2], status, i;
    long long flags = 0;
    char *child[2] , *par[2];
    char *ltproot;

    flags |= CLONE_NEWNS;
    flags |= CLONE_NEWNET;

#if ! HAVE_UNSHARE
    tst_resm(TCONF, "System doesn't support unshare.");
    tst_exit();
#endif
    
    /* Checking for Kernel Version */
    if (tst_kvercmp(2,6,19) < 0)
	return 1;

    ltproot = getenv("LTPROOT");
    if (! ltproot) {
        tst_resm(TINFO, "LTPROOT env variable is not set\n");
        tst_resm(TINFO, "Please set LTPROOT and re-run the test.. Thankyou\n");
        return -1;
    }

    child[0] = malloc (FILENAME_MAX);
    child[1] = malloc (FILENAME_MAX);
    par[0] = malloc (FILENAME_MAX);
    par[1] = malloc (FILENAME_MAX);
    if (child[0] == NULL || child[1] == NULL || \
	par[0] == NULL || par[1] == NULL)
    {
        	tst_resm(TFAIL, "error while allocating mem");
        	exit(1);
    }

    sprintf(child[0], "%s/testcases/bin/child_1.sh" , ltproot);
    sprintf(child[1], "%s/testcases/bin/child_2.sh" , ltproot);
    sprintf(par[0], "%s/testcases/bin/parent_1.sh" , ltproot);
    sprintf(par[1], "%s/testcases/bin/parent_2.sh" , ltproot);

    /* Loop for creating two child Network Namespaces */
    for(i=0;i<2;i++) {

        if ((pid[i] = fork()) == 0) {
		/* Child1 and Child2 based on the iteration. */

#if HAVE_UNSHARE
		ret = unshare(flags);
		if (ret < 0) {
			perror("Unshare");
			tst_resm(TFAIL, "Error:Unshare syscall failed for network namespace\n");
			return ret;
		}
#endif
	    return crtchild(child[i]);
        }
        else{
            //Parent

            ret = system(par[i]);
            status = WEXITSTATUS(ret);
            if (ret == -1 || status != 0) {
                tst_resm(TFAIL, "Error while running the scripts\n");
                exit(status);
            }
        }
    } //End of FOR Loop

    /* Parent waiting for two children to quit */
    for(i=0;i<2;i++) {
        ret = waitpid(pid[i], &status,__WALL);
        status = WEXITSTATUS(status);
        if (status != 0 || ret == -1){
            tst_resm(TFAIL,"waitpid() returns %d, errno %d\n", ret, status);
            fflush(stdout);
            exit(status);
        }
    }
    exit(0);
}
