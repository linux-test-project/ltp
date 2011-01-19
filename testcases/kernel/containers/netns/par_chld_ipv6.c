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
***************************************************************************/
/*=========================================================================
* This testcase creates the network namespace.
* It creates veth pair . Also assigns IP addresses to the childNS.
* Also it starts the sshd daemon @ port 7890
*
* Scripts Used: paripv6.sh childipv6.sh
*
* Author: Veerendra C <vechandr@in.ibm.com>
*                      31/07/2008
=========================================================================*/

#include <sys/utsname.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include "libclone.h"
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "config.h"

extern int crtchild(char *);

char *TCID = "netns_ipv6";
int TST_TOTAL = 1;

int main()
{
    int pid, status=0, ret ;
    long int flags = 0;
    char *ltproot, *par, *child;

    flags |= CLONE_NEWNS;
    flags |= CLONE_NEWNET;

    if (tst_kvercmp(2,6,19) < 0)
	return 1;

    ltproot = getenv("LTPROOT");

    if (! ltproot) {
        tst_resm(TINFO,"LTPROOT env variable is not set\n");
        tst_resm(TINFO,"Please set LTPROOT and re-run the test.. Thankyou\n");
        return -1;
    }

    par = malloc (FILENAME_MAX);
    child = malloc (FILENAME_MAX);

    if (par == NULL || child == NULL) {
        tst_resm(TFAIL, "error while allocating mem");
        exit(1);
    }
    sprintf(par, "%s/testcases/bin/paripv6.sh", ltproot);
    sprintf(child, "%s/testcases/bin/childipv6.sh", ltproot);

    if ((pid = fork()) == 0) {

        // Child.
#if HAVE_UNSHARE
        ret = unshare(flags);
        if (ret < 0) {
            perror("unshare");
	    tst_resm(TFAIL, "Error:Unshare syscall failed for network namespace\n");
            return 1;
        }
#else
	tst_resm(TCONF, "System doesn't have unshare support");
#endif
    return crtchild(child);
    }
    else{

        //parent
        ret = system(par);
        status = WEXITSTATUS(ret);
        if (ret == -1 || status != 0) {
            tst_resm(TFAIL, "Error: While running the IPv6 tests between \
parent & child NS\n");
            fflush(stdout);
            exit(1);
        }
    fflush(stdout);

    ret = waitpid(pid, &status, __WALL);
    status = WEXITSTATUS(status);
    if (status != 0 || ret == -1) {
        tst_resm(TFAIL, "waitpid() returns %d, errno %d\n", ret, errno);
        status =  errno;
    }
    return status;
    }
}