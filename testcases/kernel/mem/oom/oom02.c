/*
 * Out Of Memory (OOM) for mempolicy - need NUMA system support
 *
 * The program is designed to cope with unpredictable like amount and
 * system physical memory, swap size and other VMM technology like KSM,
 * memcg, memory hotplug and so on which may affect the OOM
 * behaviours. It simply increase the memory consumption 3G each time
 * until all the available memory is consumed and OOM is triggered.
 *
 * Copyright (C) 2010  Red Hat, Inc.
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include "numa_helper.h"
#include "test.h"
#include "mem.h"

char *TCID = "oom02";
int TST_TOTAL = 1;

#if HAVE_NUMA_H && HAVE_LINUX_MEMPOLICY_H && HAVE_NUMAIF_H \
	&& HAVE_MPOL_CONSTANTS

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

#if __WORDSIZE == 32
	tst_brkm(TCONF, NULL, "test is not designed for 32-bit system.");
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		tst_resm(TINFO, "OOM on MPOL_BIND mempolicy...");
		testoom(MPOL_BIND, 0, ENOMEM, 1);

		tst_resm(TINFO, "OOM on MPOL_INTERLEAVE mempolicy...");
		testoom(MPOL_INTERLEAVE, 0, ENOMEM, 1);

		tst_resm(TINFO, "OOM on MPOL_PREFERRED mempolicy...");
		testoom(MPOL_PREFERRED, 0, ENOMEM, 1);
	}
	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_require_root();
	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	if (!is_numa(NULL, NH_MEMS, 2))
		tst_brkm(TCONF, NULL, "The case need a NUMA system.");

	overcommit = get_sys_tune("overcommit_memory");
	set_sys_tune("overcommit_memory", 1, 1);
}

void cleanup(void)
{
	set_sys_tune("overcommit_memory", overcommit, 0);
}

#else /* no NUMA */
int main(void)
{
	tst_brkm(TCONF, NULL, "no NUMA development packages installed.");
}
#endif
