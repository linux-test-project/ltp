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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/* Author: Miao Xie <miaox@cn.fujitsu.com>                                    */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>

#include "../cpuset_lib/bitmask.h"
#include "../cpuset_lib/cpuset.h"
#include "../cpuset_lib/common.h"
#include "../cpuset_lib/cpuinfo.h"

/*
 * check sched domains in system
 * 
 * return 0  - sched domains is true
 *        1  - sched domains is wrong, and print error info
 *        -1 - other error
 */
int check_sched_domains(void)
{
	int i;
	char buf1[128], buf2[128];
	struct bitmask *alldomains = NULL;
	int ret = 0;

	/* get the bitmask's len */
	if (!cpus_nbits) {
		cpus_nbits = cpuset_cpus_nbits();
		if (cpus_nbits <= 0) {
			warnx("get cpus nbits failed.");
			return -1;
		}
	}

	if (getcpuinfo()) {
		warnx("getcpuinfo failed");
		return -1;
	}

	if (partition_domains()) {
		warnx("partition domains failed.");
		return -1;
	}

	alldomains = bitmask_alloc(cpus_nbits);
	if (alldomains == NULL) {
		warnx("alloc alldomains space failed.");
		return -1;
	}
	
	for (i = 0; i < ndomains; i++) {
		unsigned int cpu;
		bitmask_or(alldomains, alldomains, domains[i]);

		for (cpu = bitmask_first(domains[i]);
		     cpu < bitmask_nbits(domains[i]);
		     cpu = bitmask_next(domains[i], cpu + 1)) {
			if (bitmask_weight(domains[i]) == 1) {
				if (cpus[cpu].sched_domain != NULL) {
					ret = 1;
				    	bitmask_displaylist(buf1, sizeof(buf1),
							domains[i]);
					bitmask_displaylist(buf2, sizeof(buf2),
							cpus[cpu].sched_domain);
					warnx("cpu%d's sched domain is not "
						"NULL(Domain: %s, "
						"CPU's Sched Domain: %s).\n",
						cpu, buf1, buf2);
					goto err;
				}
				break;
			}
			if (!bitmask_equal(domains[i],
			    cpus[cpu].sched_domain)) {
			    	bitmask_displaylist(buf1, sizeof(buf1),
						domains[i]);
				bitmask_displaylist(buf2, sizeof(buf2),
				    cpus[cpu].sched_domain);
				warnx("cpu%d's sched domain is wrong"
					"(Domain: %s, "
					"CPU's Sched Domain: %s).\n",
					cpu, buf1, buf2);
				ret = 1;
				goto err;
			}
		}
	}

	for (i = 0; i < ncpus; i++) {
		if (bitmask_isbitset(alldomains, i))
			continue;
		if (cpus[i].sched_domain) {
			warnx("cpu%d has redundant sched domain", i);
			ret = 1;
			goto err;
		}
	}

err:
	bitmask_free(alldomains);
	return ret;
}

int main(void)
{
	int ret = 0;

	ret = check_sched_domains();

	return ret;
}
