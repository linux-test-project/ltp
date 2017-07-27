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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Veerendra C <vechandr@in.ibm.com>
 *
 * Net namespaces were introduced around 2.6.25.  Kernels before that,
 * assume they are not enabled.  Kernels after that, check for -EINVAL
 * when trying to use CLONE_NEWNET and CLONE_NEWNS.
 ***************************************************************************/

#define _GNU_SOURCE
#include <sched.h>
#include "config.h"
#include "libclone.h"
#include "lapi/syscalls.h"
#include "test.h"
#include "safe_macros.h"

#ifndef CLONE_NEWNS
#define CLONE_NEWNS -1
#endif

static void check_iproute(unsigned int spe_ipver)
{
	FILE *ipf;
	int n;
	unsigned int ipver = 0;

	ipf = popen("ip -V", "r");
	if (ipf == NULL)
		tst_brkm(TCONF, NULL,
				"Failed while opening pipe for iproute check");

	n = fscanf(ipf, "ip utility, iproute2-ss%u", &ipver);
	if (n < 1) {
		tst_brkm(TCONF, NULL,
			"Failed while obtaining version for iproute check");
	}
	if (ipver < spe_ipver) {
		tst_brkm(TCONF, NULL, "The commands in iproute tools do "
			"not support required objects");
	}

	pclose(ipf);
}

static int dummy(void *arg)
{
	(void) arg;
	return 0;
}

static void check_netns(void)
{
	int pid, status;
	/* Checking if the kernel supports unshare with netns capabilities. */
	if (CLONE_NEWNS == -1)
		tst_brkm(TCONF | TERRNO, NULL, "CLONE_NEWNS (%d) not supported",
			 CLONE_NEWNS);

	pid = do_clone_unshare_test(T_UNSHARE, CLONE_NEWNET | CLONE_NEWNS,
	                            dummy, NULL);
	if (pid == -1)
		tst_brkm(TCONF | TERRNO, NULL,
				"unshare syscall smoke test failed");

	SAFE_WAIT(NULL, &status);
}
