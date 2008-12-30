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
* It creates veth pair veth8 & veth9. Also assigns IP addresses to the childNS.
* Also it starts the sshd daemon @ port 7890
*
* Scripts Used: parentns.sh childns.sh
*
* Author:      Veerendra <veeren@linux.vnet.ibm.com>
=========================================================================*/

#include <sys/utsname.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "libclone.h"
#include "test.h"

int TST_TOTAL = 1;

extern pid_t getpgid(pid_t pid);
extern pid_t getsid(pid_t pid);

int crtchild(char *s1 , char *s2)
{
    char *cmd[] = { "--", s1, s2, (char *)0 };
    execve("/bin/bash", cmd, __environ);
    printf("The code would not reach here on success\n");
    perror("execve");
    return 1;
}

int create_net_namespace(char *p1, char *c1)
{
    int pid, status = 0, ret;
    long int flags = 0;
    char *ltproot, *par, *child;

    flags |= CLONE_NEWNS;
    flags |= CLONE_NEWNET;

    if (tst_kvercmp(2,6,19) < 0)
        return 1;

    ltproot = getenv("LTPROOT");

    if ( !ltproot) {
        printf("LTPROOT env variable is not set\n");
        printf("Please set LTPROOT and re-run the test.. Thankyou\n");
        return -1;
    }

    par = malloc(FILENAME_MAX);
    child = malloc(FILENAME_MAX);

    if (par == NULL || child == NULL ) {
                printf("FAIL: error while allocating memory");
                exit(1);
        }

    sprintf(par, "%s/testcases/kernel/containers/netns/parentns.sh %s" , ltproot, p1);
    sprintf(child, "%s/testcases/kernel/containers/netns/childns.sh" , ltproot);

    if ((pid = fork()) == 0) {

        // Child.
        ret = unshare(flags);
        if (ret < 0) {
            perror("unshare");
	    printf ("Error:Unshare syscall failed for network namespace\n");
            return 1;
        }
    return crtchild(child, c1);
    }
    else{

        //parent
        ret = system(par);
        status = WEXITSTATUS(ret);
        if ( ret == -1 || status != 0) {
            printf("Error while running the script\n");
            fflush(stdout);
            exit(1);
        }
        fflush(stdout);

        ret = waitpid(pid, &status, __WALL);
        status = WEXITSTATUS(status);
        if ( ret  == -1 || status != 0)
            printf("Error: waitpid() returns %d, status %d\n", ret, status);

    }
    return status;
}
