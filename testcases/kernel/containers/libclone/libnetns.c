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
static int child_fn(void *c1);

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
	char *ltproot, *par;
	long int clone_flags = 0;
	int stack_size = getpagesize() * 4;
	void *childstack, *stack;

	if (tst_kvercmp(2, 6, 19) < 0)
		return 1;

	stack = malloc(stack_size);
	if (!stack) {
		perror("failled to malloc memory for stack...");
		return -1;
	}
	childstack = stack + stack_size;

	clone_flags |= CLONE_NEWNS;
/* Enable other namespaces too optionally */
#ifdef CLONE_NEWPID
	clone_flags |= CLONE_NEWPID;
#endif

#ifdef __ia64__
	pid = clone2(child_fn, childstack, getpagesize(), clone_flags | SIGCHLD,
						(void *)c1, NULL, NULL, NULL);
#else
	pid = clone(child_fn, childstack, clone_flags | SIGCHLD, (void *)c1);
#endif

	if (pid == -1) {
		perror("Failled to do clone...");
		free(stack);
		return -1;
	}

/* This code will be executed in parent */
    ltproot = getenv("LTPROOT");

    if ( !ltproot) {
        printf("LTPROOT env variable is not set\n");
        printf("Please set LTPROOT and re-run the test.. Thankyou\n");
        return -1;
    }

    par = malloc(FILENAME_MAX);

    if (par == NULL) {
                printf("FAIL: error while allocating memory");
                exit(1);
        }

	/* We need to pass the child pid to the parentns.sh script */
    sprintf(par, "%s/testcases/kernel/containers/netns/parentns.sh %s %u",
							ltproot, p1, pid);

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

    return status;
}

/* The function to be executed in the child namespace */
int child_fn(void *c1)
{
	char *ltproot, *child;
	unsigned long flags = 0;
	int ret;

/* Flags to unshare different Namespaces */
	flags |= CLONE_NEWNS;
	flags |= CLONE_NEWNET;
	flags |= CLONE_NEWUTS;
	flags |= CLONE_FS;

	ltproot = getenv("LTPROOT");

	if (!ltproot) {
		printf("LTPROOT env variable is not set\n");
		printf("Please set LTPROOT and re-run the test.. Thankyou\n");
		return -1;
	}

	child = malloc(FILENAME_MAX);
	if (child == NULL) {
		printf("FAIL: error while allocating memory");
		exit(1);
	}

	sprintf(child, "%s/testcases/kernel/containers/netns/childns.sh",
								 ltproot);

	/* Unshare the network namespace in the child */
	ret = unshare(flags);
	if (ret < 0) {
		perror("Failled to unshare for netns...");
		return 1;
	}
	return crtchild(child, c1);
}
