// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Red Hat, Inc.
 * Copyright (c) 2020 Li Wang <liwang@redhat.com>
 */

/*
 * Tests tst_cgroup.h APIs
 */

#include "tst_test.h"
#include "tst_cgroup.h"

#define PATH_CGROUP1 "/mnt/liwang1"
#define PATH_CGROUP2 "/mnt/liwang2"
#define MEMSIZE 1024 * 1024

static void do_test(void)
{
	pid_t pid = SAFE_FORK();

	switch (pid) {
	case 0:
		tst_cgroup_move_current(PATH_CGROUP1);
		tst_cgroup_mem_set_maxbytes(PATH_CGROUP1, MEMSIZE);
		tst_cgroup_mem_set_maxswap(PATH_CGROUP1, MEMSIZE);

		tst_cgroup_move_current(PATH_CGROUP2);

	break;
	default:
		tst_cgroup_move_current(PATH_TMP_CG_CST);

		tst_cgroup_move_current(PATH_TMP_CG_MEM);
		tst_cgroup_mem_set_maxbytes(PATH_TMP_CG_MEM, MEMSIZE);
		tst_cgroup_mem_set_maxswap(PATH_TMP_CG_MEM, MEMSIZE);
	break;
	}

	tst_res(TPASS, "Cgroup mount test");
}

static void setup(void)
{
	tst_cgroup_mount(TST_CGROUP_MEMCG, PATH_TMP_CG_MEM);
	tst_cgroup_mount(TST_CGROUP_MEMCG, PATH_CGROUP1);

	tst_cgroup_mount(TST_CGROUP_CPUSET, PATH_TMP_CG_CST);
	tst_cgroup_mount(TST_CGROUP_CPUSET, PATH_CGROUP2);
}

static void cleanup(void)
{
	tst_cgroup_umount(PATH_TMP_CG_MEM);
	tst_cgroup_umount(PATH_CGROUP1);

	tst_cgroup_umount(PATH_TMP_CG_CST);
	tst_cgroup_umount(PATH_CGROUP2);
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
