// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Sanity checks for the per-CPU topology attributes exported under
 * /sys/devices/system/cpu/cpuN/topology/.
 *
 * The topology masks describe how a CPU relates to the others. They are nested
 * from the smallest (SMT threads) to the largest (package) group, so for every
 * online CPU the test verifies that:
 *
 * - thread_siblings_list is a subset of core_cpus_list
 * - core_cpus_list is a subset of package_cpus_list
 * - package_cpus_list is a subset of the present CPUs, not just the online
 *   ones: the whole topology/ directory (package_cpus_list included) is
 *   added/removed per-CPU on hotplug (drivers/base/topology.c registers it
 *   against a cpuhp state), so it only ever lists CPUs that share the
 *   package and are also online right now. Comparing against ``present``
 *   instead of ``online`` avoids relying on that always being reliably kept
 *   in sync on every architecture, and avoids a spurious failure if a
 *   sibling CPU is hot-unplugged concurrently with this test running.
 * - the CPU itself is contained in its own thread_siblings_list
 * - core_id is in [0, highest possible CPU id], since there can be at most
 *   as many cores as possible CPUs (every core has at least one thread)
 * - physical_package_id is in [-1, highest possible CPU id] on physical
 *   hardware, or just at least -1 under virtualization (see below). -1 is a
 *   legitimate value here, not just a range extension for safety: per
 *   include/linux/topology.h, -1 is the architecture-neutral default for
 *   topology_physical_package_id() (and die_id/cluster_id/book_id/
 *   drawer_id) on any architecture, or any CPU type within one, that does
 *   not implement package detection. topology_core_id() does not have this
 *   documented -1 fallback (its generic default is 0), so core_id is not
 *   given the same allowance.
 *
 *   The upper bound on physical_package_id is dropped under virtualization:
 *   store_cpu_topology() in drivers/base/arch_topology.c falls back to
 *   physical_package_id = cpu_to_node(cpu) whenever ACPI/DT topology
 *   parsing did not already populate it (common on minimal/generic arm64 VM
 *   firmware without full ACPI PPTT tables). On physical hardware this
 *   fallback is still bounded by the number of possible CPUs, since NUMA
 *   node ids are compacted/renumbered from the firmware's proximity domains
 *   and there can never be more NUMA nodes than CPUs (each node needs at
 *   least one). But a VM's NUMA node numbering is a hypervisor/firmware
 *   policy choice entirely unrelated to its own CPU count, e.g. mirroring a
 *   large host's own (possibly sparse) NUMA node ids under NUMA
 *   passthrough/vNUMA configuration, so the bound would not be meaningful
 *   there and is dropped to avoid a false failure.
 *
 * All checks skip gracefully with TCONF when a particular attribute is not
 * present, as the exact set of topology files differs between kernel versions
 * and architectures.
 */

#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_sysfs_assert.h"
#include "tst_path_defs.h"

static void check_self_in_threads(int cpu)
{
	TST_SYSFS_ASSERT_LIST_CONTAINS(cpu,
		PATH_SYS_CPU "/cpu%d/topology/thread_siblings_list", cpu);
}

static void check_cpu_topology(int cpu, int poss_max_id, long pkg_max_id)
{
	char sub[PATH_MAX], super[PATH_MAX];

	if (access(PATH_SYS_CPU, F_OK))
		return;

	if (!tst_sysfs_exists(PATH_SYS_CPU "/cpu%d/topology/core_id", cpu)) {
		tst_res(TCONF, "cpu%d has no topology directory", cpu);
		return;
	}

	tst_res(TINFO, "Checking cpu%d topology", cpu);

	TST_SYSFS_ASSERT_RANGELL(0, poss_max_id,
			       PATH_SYS_CPU "/cpu%d/topology/core_id", cpu);

	TST_SYSFS_ASSERT_RANGELL(-1, pkg_max_id,
			       PATH_SYS_CPU "/cpu%d/topology/physical_package_id",
			       cpu);

	snprintf(sub, sizeof(sub),
		 PATH_SYS_CPU "/cpu%d/topology/thread_siblings_list", cpu);
	snprintf(super, sizeof(super),
		 PATH_SYS_CPU "/cpu%d/topology/core_cpus_list", cpu);
	TST_SYSFS_ASSERT_LIST_SUBSET(sub, super);

	snprintf(sub, sizeof(sub),
		 PATH_SYS_CPU "/cpu%d/topology/core_cpus_list", cpu);
	snprintf(super, sizeof(super),
		 PATH_SYS_CPU "/cpu%d/topology/package_cpus_list", cpu);
	TST_SYSFS_ASSERT_LIST_SUBSET(sub, super);

	snprintf(sub, sizeof(sub),
		 PATH_SYS_CPU "/cpu%d/topology/package_cpus_list", cpu);
	TST_SYSFS_ASSERT_LIST_SUBSET(sub, PATH_SYS_CPU "/present");

	check_self_in_threads(cpu);
}

static void do_test(void)
{
	int count, max_id, poss_count, poss_max_id, cpu;
	long pkg_max_id;

	if (TST_SYSFS_ASSERT_PARSE_LIST(&count, &max_id, PATH_SYS_CPU "/online"))
		return;

	if (TST_SYSFS_ASSERT_PARSE_LIST(&poss_count, &poss_max_id,
					PATH_SYS_CPU "/possible"))
		return;

	if (tst_is_virt(VIRT_ANY))
		pkg_max_id = LONG_MAX;
	else
		pkg_max_id = poss_max_id;

	for (cpu = 0; cpu <= max_id; cpu++)
		check_cpu_topology(cpu, poss_max_id, pkg_max_id);
}

static struct tst_test test = {
	.test_all = do_test,
};
