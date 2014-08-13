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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#define _GNU_SOURCE

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
#include "test.h"
#include "libclone.h"
#include "config.h"
#include "common.h"
#include "netns_helper.h"

char *TCID = "netns_2children";
int TST_TOTAL = 1;

static void setup(void)
{
	tst_require_root(NULL);
	check_iproute();
	check_netns();
}

int main(void)
{
	int ret, pid[2], status, i;
	long long flags = 0;
	char child[2][FILENAME_MAX], par[2][FILENAME_MAX];

	setup();

	flags |= CLONE_NEWNS;
	flags |= CLONE_NEWNET;

#if ! HAVE_UNSHARE
	tst_resm(TCONF, "System doesn't support unshare.");
	tst_exit();
#endif

	/* Checking for Kernel Version */
	if (tst_kvercmp(2, 6, 19) < 0)
		tst_brkm(TCONF, NULL, "CLONE_NEWPID not supported");

	strcpy(child[0], "child_1.sh");
	strcpy(child[1], "child_2.sh");
	strcpy(par[0], "parent_1.sh");
	strcpy(par[1], "parent_2.sh");

	/* Loop for creating two child Network Namespaces */
	for (i = 0; i < 2; i++) {

		if ((pid[i] = fork()) == 0) {
			/* Child1 and Child2 based on the iteration. */

#if HAVE_UNSHARE
			ret = unshare(flags);
			if (ret < 0) {
				perror("Unshare");
				tst_resm(TFAIL,
					 "Error:Unshare syscall failed for network namespace");
				tst_exit();
			}
#endif
			if (crtchild(child[i], NULL) != 0) {
				tst_resm(TFAIL, "Failed running child script");
				tst_exit();
			}
		} else {
			//Parent

			ret = system(par[i]);
			status = WEXITSTATUS(ret);
			if (ret == -1 || status != 0) {
				tst_resm(TFAIL,
					 "Error while running the scripts");
				tst_exit();
			}
		}
	}			//End of FOR Loop

	/* Parent waiting for two children to quit */
	for (i = 0; i < 2; i++) {
		ret = waitpid(pid[i], &status, __WALL);
		status = WEXITSTATUS(status);
		if (status != 0 || ret == -1) {
			tst_resm(TFAIL, "waitpid() returns %d, errno %d", ret,
				 status);
			fflush(stdout);
			tst_exit();
		}
	}

	tst_resm(TPASS, "two children ns");
	tst_exit();
}
