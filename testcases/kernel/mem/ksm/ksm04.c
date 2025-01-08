// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2010-2017  Red Hat, Inc.
 */

/*\
 * [Description]
 *
 * Prerequisites:
 *
 * ksm and ksmtuned daemons need to be disabled. Otherwise, it could
 * distrub the testing as they also change some ksm tunables depends
 * on current workloads.
 *
 * [Algorithm]
 *
 * - Check ksm feature and backup current run setting.
 * - Change run setting to 1 - merging.
 * - 3 memory allocation programs have the memory contents that 2 of
 *   them are all 'a' and one is all 'b'.
 * - Check ksm statistics and verify the content.
 * - 1 program changes the memory content from all 'a' to all 'b'.
 * - Check ksm statistics and verify the content.
 * - All programs change the memory content to all 'd'.
 * - Check ksm statistics and verify the content.
 * - Change one page of a process.
 * - Check ksm statistics and verify the content.
 * - Change run setting to 2 - unmerging.
 * - Check ksm statistics and verify the content.
 * - Change run setting to 0 - stop.
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "mem.h"
#include "ksm_common.h"

#ifdef HAVE_NUMA_V2
#include <numaif.h>

static void verify_ksm(void)
{
	unsigned long nmask[MAXNODES / BITS_PER_LONG] = { 0 };
	unsigned int node;

	node = get_a_numa_node();
	set_node(nmask, node);

	SAFE_CG_PRINTF(tst_cg, "memory.max", "%lu", TESTMEM);

	if (set_mempolicy(MPOL_BIND, nmask, MAXNODES) == -1) {
		if (errno != ENOSYS)
			tst_brk(TBROK | TERRNO, "set_mempolicy");
		else
			tst_brk(TCONF, "set_mempolicy syscall is not "
					"implemented on your system.");
	}
	create_same_memory(size, num, unit);

	write_cpusets(tst_cg, node);
	create_same_memory(size, num, unit);
}

static void setup(void)
{
	parse_ksm_options(opt_sizestr, &size, opt_numstr, &num, opt_unitstr, &unit);

	SAFE_CG_PRINTF(tst_cg, "cgroup.procs", "%d", getpid());

	if (opt_sizestr && size > DEFAULT_MEMSIZE)
		tst_set_timeout(32 * (size / DEFAULT_MEMSIZE));
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{"n:", &opt_numstr,  "Number of processes"},
		{"s:", &opt_sizestr, "Memory allocation size in MB"},
		{"u:", &opt_unitstr, "Memory allocation unit in MB"},
		{}
	},
	.setup = setup,
	.save_restore = (const struct tst_path_val[]) {
		{"/sys/kernel/mm/ksm/run", NULL, TST_SR_TBROK},
		{"/sys/kernel/mm/ksm/sleep_millisecs", NULL, TST_SR_TBROK},
		{"/sys/kernel/mm/ksm/max_page_sharing", NULL,
			TST_SR_SKIP_MISSING | TST_SR_TCONF_RO},
		{"/sys/kernel/mm/ksm/merge_across_nodes", "1",
			TST_SR_SKIP_MISSING | TST_SR_TCONF_RO},
		{"/sys/kernel/mm/ksm/smart_scan", "0",
			TST_SR_SKIP_MISSING | TST_SR_TBROK_RO},
		{}
	},
	.needs_kconfigs = (const char *const[]){
		"CONFIG_KSM=y",
		NULL
	},
	.test_all = verify_ksm,
	.timeout = 32,
	.needs_cgroup_ctrls = (const char *const []){
		"memory", "cpuset", NULL
	},
};

#else
	TST_TEST_TCONF(NUMA_ERROR_MSG);
#endif
