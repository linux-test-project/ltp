/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2009 FUJITSU LIMITED                                         */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/* Author: Miao Xie <miaox@cn.fujitsu.com>                                    */
/*                                                                            */
/******************************************************************************/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>

#include "test.h"

#include "../cpuset_lib/bitmask.h"
#include "../cpuset_lib/cpuset.h"
#include "../cpuset_lib/common.h"
#include "../cpuset_lib/cpuinfo.h"

char *TCID = "cpuset_check_domains";
int TST_TOTAL = 1;

#ifdef HAVE_LINUX_MEMPOLICY_H

/*
 * check sched domains in system
 *
 * return 0  - sched domains is true
 *        1  - sched domains is wrong, and print error info
 *        -1 - other error
 */
void check_sched_domains(void)
{
	int i;
	char buf1[128], buf2[128];
	struct bitmask *alldomains = NULL;

	/* get the bitmask's len */
	if (!cpus_nbits) {
		cpus_nbits = cpuset_cpus_nbits();
		if (cpus_nbits <= 0) {
			tst_resm(TFAIL, "get cpus nbits failed");
			return;
		}
	}

	if (getcpuinfo()) {
		tst_resm(TFAIL, "getcpuinfo failed");
		return;
	}

	if (partition_domains()) {
		tst_resm(TFAIL, "partition domains failed.");
		return;
	}

	alldomains = bitmask_alloc(cpus_nbits);
	if (alldomains == NULL) {
		tst_resm(TFAIL, "alloc alldomains space failed.");
		return;
	}

	for (i = 0; i < ndomains; i++) {
		unsigned int cpu;
		bitmask_or(alldomains, alldomains, domains[i]);

		for (cpu = bitmask_first(domains[i]);
		     cpu < bitmask_nbits(domains[i]);
		     cpu = bitmask_next(domains[i], cpu + 1)) {
			if (bitmask_weight(domains[i]) == 1) {
				if (cpus[cpu].sched_domain != NULL) {
					bitmask_displaylist(buf1, sizeof(buf1),
							    domains[i]);
					bitmask_displaylist(buf2, sizeof(buf2),
							    cpus[cpu].
							    sched_domain);
					tst_resm(TFAIL,
						 "cpu%d's sched domain is not "
						 "NULL(Domain: %s, "
						 "CPU's Sched Domain: %s).",
						 cpu, buf1, buf2);
					goto err;
				}
				break;
			}
			if (!bitmask_equal(domains[i], cpus[cpu].sched_domain)) {
				bitmask_displaylist(buf1, sizeof(buf1),
						    domains[i]);
				bitmask_displaylist(buf2, sizeof(buf2),
						    cpus[cpu].sched_domain);
				tst_resm(TFAIL, "cpu%d's sched domain is wrong"
					 "(Domain: %s, CPU's Sched Domain: %s).",
					 cpu, buf1, buf2);
				goto err;
			}
		}
	}

	for (i = 0; i < ncpus; i++) {
		if (bitmask_isbitset(alldomains, i))
			continue;
		if (cpus[i].sched_domain) {
			tst_resm(TFAIL, "cpu%d has redundant sched domain", i);
			goto err;
		}
	}

	tst_resm(TPASS, "check_sched_domains passed");

err:
	bitmask_free(alldomains);
}

int main(void)
{
	check_sched_domains();
	tst_exit();
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL,
		 "System doesn't have required mempolicy support");
}
#endif
