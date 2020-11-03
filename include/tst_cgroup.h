// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Red Hat, Inc.
 * Copyright (c) 2020 Li Wang <liwang@redhat.com>
 */

#ifndef TST_CGROUP_H
#define TST_CGROUP_H

#define PATH_TMP_CG_MEM	"/tmp/cgroup_mem"
#define PATH_TMP_CG_CST	"/tmp/cgroup_cst"

enum tst_cgroup_ver {
	TST_CGROUP_V1 = 1,
	TST_CGROUP_V2 = 2,
};

enum tst_cgroup_ctrl {
	TST_CGROUP_MEMCG = 1,
	TST_CGROUP_CPUSET = 2,
	/* add cgroup controller */
};

enum tst_cgroup_ver tst_cgroup_version(void);

/* To mount/umount specified cgroup controller on 'cgroup_dir' path */
void tst_cgroup_mount(enum tst_cgroup_ctrl ctrl, const char *cgroup_dir);
void tst_cgroup_umount(const char *cgroup_dir);

/* To move current process PID to the mounted cgroup tasks */
void tst_cgroup_move_current(const char *cgroup_dir);

/* To set cgroup controller knob with new value */
void tst_cgroup_set_knob(const char *cgroup_dir, const char *knob, long value);

/* Set of functions to set knobs under the memory controller */
void tst_cgroup_mem_set_maxbytes(const char *cgroup_dir, long memsz);
int  tst_cgroup_mem_swapacct_enabled(const char *cgroup_dir);
void tst_cgroup_mem_set_maxswap(const char *cgroup_dir, long memsz);

/* Set of functions to read/write cpuset controller files content */
void tst_cgroup_cpuset_read_files(const char *cgroup_dir, const char *filename,
	char *retbuf, size_t retbuf_sz);
void tst_cgroup_cpuset_write_files(const char *cgroup_dir, const char *filename,
	const char *buf);

#endif /* TST_CGROUP_H */
