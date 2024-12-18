// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2011-2021
 * Copyright (c) Cyril Hrubis <chrubis@suse.cz> 2024
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_cgroup.h"
#include "numa_helper.h"

static void gather_node_cpus(char *cpus, long nd)
{
	int ncpus = 0;
	int i;
	long online;
	char buf[BUFSIZ];
	char path[BUFSIZ], path1[BUFSIZ];

	while (tst_path_exists(PATH_SYS_SYSTEM "/cpu/cpu%d", ncpus))
		ncpus++;

	for (i = 0; i < ncpus; i++) {
		snprintf(path, BUFSIZ,
			 PATH_SYS_SYSTEM "/node/node%ld/cpu%d", nd, i);
		if (tst_path_exists("%s", path)) {
			snprintf(path1, BUFSIZ, "%s/online", path);
			/*
			 * if there is no online knob, then the cpu cannot
			 * be taken offline
			 */
			if (tst_path_exists("%s", path1)) {
				SAFE_FILE_SCANF(path1, "%ld", &online);
				if (online == 0)
					continue;
			}
			sprintf(buf, "%d,", i);
			strcat(cpus, buf);
		}
	}
	/* Remove the trailing comma. */
	cpus[strlen(cpus) - 1] = '\0';
}

void write_node_cpusets(const struct tst_cg_group *cg, long nd)
{
	char cpus[BUFSIZ] = "";

	SAFE_CG_PRINTF(cg, "cpuset.mems", "%ld", nd);

	gather_node_cpus(cpus, nd);
	/*
	 * If the 'nd' node doesn't contain any CPUs,
	 * the first ID of CPU '0' will be used as
	 * the value of cpuset.cpus.
	 */
	if (strlen(cpus) != 0) {
		SAFE_CG_PRINT(cg, "cpuset.cpus", cpus);
	} else {
		tst_res(TINFO, "No CPUs in the node%ld; "
				"using only CPU0", nd);
		SAFE_CG_PRINT(cg, "cpuset.cpus", "0");
	}
}
