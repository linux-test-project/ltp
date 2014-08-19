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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
***************************************************************************/
/*=========================================================================
* This testcase creates the network namespace.
* It creates veth pair veth8 & veth9. Also assigns IP addresses to the childNS.
* Also it starts the sshd daemon @ port 7890
*
* Scripts Used: netns_parentns.sh netns_childns.sh
*
* Author:	  Veerendra <veeren@linux.vnet.ibm.com>
=========================================================================*/

#define _GNU_SOURCE

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
#include <inttypes.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "libclone.h"
#include "test.h"
#include "config.h"
#include "common.h"

#define PARENTNS_SCRIPT "netns_parentns.sh"
#define CHILDNS_SCRIPT "netns_childns.sh"

static int child_fn(void *c1);

int crtchild(char *s1, char *s2)
{
	char *cmd[] = { "--", s1, s2, (char *)0 };
	execve("/bin/sh", cmd, __environ);
	fprintf(stderr, "Failed to execve(%s, %s): %m\n", s1, s2);
	return 1;
}

int create_net_namespace(char *p1, char *c1)
{
	int pid, status = 0, ret;
	char par[FILENAME_MAX];
	long int clone_flags = 0;

	if (tst_kvercmp(2, 6, 19) < 0)
		tst_brkm(TCONF, NULL, "CLONE_NEWPID not supported");

	clone_flags |= CLONE_NEWNS;
/* Enable other namespaces too optionally */
#ifdef CLONE_NEWPID
	clone_flags |= CLONE_NEWPID;
#endif

	pid = ltp_clone_quick(clone_flags, child_fn, (void *)c1);

	if (pid == -1) {
		perror("Failed to do clone...");
		return -1;
	}

	/* We need to pass the child pid to the parentns.sh script */
	sprintf(par, "%s %s %" PRId32, PARENTNS_SCRIPT, p1, pid);

	ret = system(par);
	status = WEXITSTATUS(ret);
	if (ret == -1 || status != 0) {
		tst_resm(TFAIL, "Error while running the script\n");
		fflush(stdout);
		tst_exit();
	}
	fflush(stdout);

	ret = waitpid(pid, &status, __WALL);
	status = WEXITSTATUS(status);
	if (ret == -1 || status != 0) {
		printf("Error: waitpid() returns %d, status %d\n", ret, status);
	}

	return status;
}

/* The function to be executed in the child namespace */
int child_fn(void *c1)
{
	unsigned long flags = 0;
#if HAVE_UNSHARE
	int ret;
#endif

	/* Flags to unshare different Namespaces */
	flags |= CLONE_NEWNS;
	flags |= CLONE_NEWNET;
	flags |= CLONE_NEWUTS;
	flags |= CLONE_FS;

	/* Unshare the network namespace in the child */
#if HAVE_UNSHARE
	ret = unshare(flags);
	if (ret < 0) {
		perror("Failed to unshare for netns...");
		return 1;
	}
	return crtchild(CHILDNS_SCRIPT, c1);
#else
	printf("System doesn't support unshare.\n");
	return -1;
#endif
}
