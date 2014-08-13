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
* It creates veth pair . Also assigns IP addresses to the childNS.
* Also it starts the sshd daemon @ port 7890
*
* Scripts Used: paripv6.sh childipv6.sh
*
* Author: Veerendra C <vechandr@in.ibm.com>
*                      31/07/2008
=========================================================================*/

#define _GNU_SOURCE

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
#include "common.h"
#include "netns_helper.h"

char *TCID = "netns_ipv6";
int TST_TOTAL = 1;

#define PARENT_SCRIPT "paripv6.sh"
#define CHILD_SCRIPT "childipv6.sh"

static void setup(void)
{
	tst_require_root(NULL);
	check_iproute();
	check_netns();
}

int main(void)
{
	int pid, status = 0, ret;
	long int flags = 0;

	setup();

	flags |= CLONE_NEWNS;
	flags |= CLONE_NEWNET;

	if (tst_kvercmp(2, 6, 19) < 0)
		tst_brkm(TCONF, NULL, "CLONE_NEWPID not supported");

	if ((pid = fork()) == 0) {

		// Child.
#if HAVE_UNSHARE
		ret = unshare(flags);
		if (ret < 0) {
			perror("unshare");
			tst_resm(TFAIL,
				 "Error:Unshare syscall failed for network namespace");
			tst_exit();
		}
#else
		tst_resm(TCONF, "System doesn't have unshare support");
#endif
		if (crtchild(CHILD_SCRIPT, NULL) != 0) {
			tst_resm(TFAIL, "Failed running child script");
			tst_exit();
		}
	} else {

		//parent
		ret = system(PARENT_SCRIPT);
		status = WEXITSTATUS(ret);
		if (ret == -1 || status != 0) {
			tst_resm(TFAIL,
				 "Error: While running the IPv6 tests between \
parent & child NS");
			fflush(stdout);
			tst_exit();
		}
		fflush(stdout);

		ret = waitpid(pid, &status, __WALL);
		status = WEXITSTATUS(status);
		if (status != 0 || ret == -1) {
			tst_resm(TFAIL, "waitpid() returns %d, errno %d", ret,
				 errno);
			tst_exit();
		}
		tst_resm(TPASS, "par child ipv6");
		tst_exit();
	}
	tst_exit();
}
