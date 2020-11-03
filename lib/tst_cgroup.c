// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Red Hat, Inc.
 * Copyright (c) 2020 Li Wang <liwang@redhat.com>
 */

#define TST_NO_DEFAULT_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <unistd.h>

#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_stdio.h"
#include "tst_cgroup.h"
#include "tst_device.h"

static enum tst_cgroup_ver tst_cg_ver;
static int clone_children;

static int tst_cgroup_check(const char *cgroup)
{
	char line[PATH_MAX];
	FILE *file;
	int cg_check = 0;

	file = SAFE_FOPEN("/proc/filesystems", "r");
	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, cgroup) != NULL) {
			cg_check = 1;
			break;
		}
	}
	SAFE_FCLOSE(file);

	return cg_check;
}

enum tst_cgroup_ver tst_cgroup_version(void)
{
        enum tst_cgroup_ver cg_ver;

        if (tst_cgroup_check("cgroup2")) {
                if (!tst_is_mounted("cgroup2") && tst_is_mounted("cgroup"))
                        cg_ver = TST_CGROUP_V1;
                else
                        cg_ver = TST_CGROUP_V2;

                goto out;
        }

        if (tst_cgroup_check("cgroup"))
                cg_ver = TST_CGROUP_V1;

        if (!cg_ver)
                tst_brk(TCONF, "Cgroup is not configured");

out:
        return cg_ver;
}

static void tst_cgroup1_mount(const char *name, const char *option,
			const char *mnt_path, const char *new_path)
{
	char knob_path[PATH_MAX];
	if (tst_is_mounted(mnt_path))
		goto out;

	SAFE_MKDIR(mnt_path, 0777);
	if (mount(name, mnt_path, "cgroup", 0, option) == -1) {
		if (errno == ENODEV) {
			if (rmdir(mnt_path) == -1)
				tst_res(TWARN | TERRNO, "rmdir %s failed", mnt_path);
			tst_brk(TCONF,
				 "Cgroup v1 is not configured in kernel");
		}
		tst_brk(TBROK | TERRNO, "mount %s", mnt_path);
	}

	/*
	 * We should assign one or more memory nodes to cpuset.mems and
	 * cpuset.cpus, otherwise, echo $$ > tasks gives “ENOSPC: no space
	 * left on device” when trying to use cpuset.
	 *
	 * Or, setting cgroup.clone_children to 1 can help in automatically
	 * inheriting memory and node setting from parent cgroup when a
	 * child cgroup is created.
	 */
	if (strcmp(option, "cpuset") == 0) {
		sprintf(knob_path, "%s/cgroup.clone_children", mnt_path);
		SAFE_FILE_SCANF(knob_path, "%d", &clone_children);
		SAFE_FILE_PRINTF(knob_path, "%d", 1);
	}
out:
	SAFE_MKDIR(new_path, 0777);

	tst_res(TINFO, "Cgroup(%s) v1 mount at %s success", option, mnt_path);
}

static void tst_cgroup2_mount(const char *mnt_path, const char *new_path)
{
	if (tst_is_mounted(mnt_path))
		goto out;

	SAFE_MKDIR(mnt_path, 0777);
	if (mount("cgroup2", mnt_path, "cgroup2", 0, NULL) == -1) {
		if (errno == ENODEV) {
			if (rmdir(mnt_path) == -1)
				tst_res(TWARN | TERRNO, "rmdir %s failed", mnt_path);
			tst_brk(TCONF,
				 "Cgroup v2 is not configured in kernel");
		}
		tst_brk(TBROK | TERRNO, "mount %s", mnt_path);
	}

out:
	SAFE_MKDIR(new_path, 0777);

	tst_res(TINFO, "Cgroup v2 mount at %s success", mnt_path);
}

static void tst_cgroupN_umount(const char *mnt_path, const char *new_path)
{
	FILE *fp;
	int fd;
	char s_new[BUFSIZ], s[BUFSIZ], value[BUFSIZ];
	char knob_path[PATH_MAX];

	if (!tst_is_mounted(mnt_path))
		return;

	/* Move all processes in task(v2: cgroup.procs) to its parent node. */
	if (tst_cg_ver & TST_CGROUP_V1)
		sprintf(s, "%s/tasks", mnt_path);
	if (tst_cg_ver & TST_CGROUP_V2)
		sprintf(s, "%s/cgroup.procs", mnt_path);

	fd = open(s, O_WRONLY);
	if (fd == -1)
		tst_res(TWARN | TERRNO, "open %s", s);

	if (tst_cg_ver & TST_CGROUP_V1)
		snprintf(s_new, BUFSIZ, "%s/tasks", new_path);
	if (tst_cg_ver & TST_CGROUP_V2)
		snprintf(s_new, BUFSIZ, "%s/cgroup.procs", new_path);

	fp = fopen(s_new, "r");
	if (fp == NULL)
		tst_res(TWARN | TERRNO, "fopen %s", s_new);
	if ((fd != -1) && (fp != NULL)) {
		while (fgets(value, BUFSIZ, fp) != NULL)
			if (write(fd, value, strlen(value) - 1)
			    != (ssize_t)strlen(value) - 1)
				tst_res(TWARN | TERRNO, "write %s", s);
	}
	if (tst_cg_ver & TST_CGROUP_V1) {
		sprintf(knob_path, "%s/cpuset.cpus", mnt_path);
		if (!access(knob_path, F_OK)) {
			sprintf(knob_path, "%s/cgroup.clone_children", mnt_path);
			SAFE_FILE_PRINTF(knob_path, "%d", clone_children);
		}
	}
	if (fd != -1)
		close(fd);
	if (fp != NULL)
		fclose(fp);
	if (rmdir(new_path) == -1)
		tst_res(TWARN | TERRNO, "rmdir %s", new_path);
	if (umount(mnt_path) == -1)
		tst_res(TWARN | TERRNO, "umount %s", mnt_path);
	if (rmdir(mnt_path) == -1)
		tst_res(TWARN | TERRNO, "rmdir %s", mnt_path);

	if (tst_cg_ver & TST_CGROUP_V1)
		tst_res(TINFO, "Cgroup v1 unmount success");
	if (tst_cg_ver & TST_CGROUP_V2)
		tst_res(TINFO, "Cgroup v2 unmount success");
}

struct tst_cgroup_path {
	char *mnt_path;
	char *new_path;
	struct tst_cgroup_path *next;
};

static struct tst_cgroup_path *tst_cgroup_paths;

static void tst_cgroup_set_path(const char *cgroup_dir)
{
	char cgroup_new_dir[PATH_MAX];
	struct tst_cgroup_path *tst_cgroup_path, *a;

	if (!cgroup_dir)
		tst_brk(TBROK, "Invalid cgroup dir, plese check cgroup_dir");

	sprintf(cgroup_new_dir, "%s/ltp_%d", cgroup_dir, rand());

	/* To store cgroup path in the 'path' list */
	tst_cgroup_path = SAFE_MALLOC(sizeof(struct tst_cgroup_path));
	tst_cgroup_path->mnt_path = SAFE_MALLOC(strlen(cgroup_dir) + 1);
	tst_cgroup_path->new_path = SAFE_MALLOC(strlen(cgroup_new_dir) + 1);
	tst_cgroup_path->next = NULL;

	if (!tst_cgroup_paths) {
		tst_cgroup_paths = tst_cgroup_path;
	} else {
		a = tst_cgroup_paths;
		do {
			if (!a->next) {
				a->next = tst_cgroup_path;
				break;
			}
			a = a->next;
		} while (a);
	}

	sprintf(tst_cgroup_path->mnt_path, "%s", cgroup_dir);
	sprintf(tst_cgroup_path->new_path, "%s", cgroup_new_dir);
}

static char *tst_cgroup_get_path(const char *cgroup_dir)
{
	struct tst_cgroup_path *a;

	if (!tst_cgroup_paths)
		return NULL;

	a = tst_cgroup_paths;

	while (strcmp(a->mnt_path, cgroup_dir) != 0){
		if (!a->next) {
			tst_res(TINFO, "%s is not found", cgroup_dir);
			return NULL;
		}
		a = a->next;
	};

	return a->new_path;
}

static void tst_cgroup_del_path(const char *cgroup_dir)
{
	struct tst_cgroup_path *a, *b;

	if (!tst_cgroup_paths)
		return;

	a = b = tst_cgroup_paths;

	while (strcmp(b->mnt_path, cgroup_dir) != 0) {
		if (!b->next) {
			tst_res(TINFO, "%s is not found", cgroup_dir);
			return;
		}
		a = b;
		b = b->next;
	};

	if (b == tst_cgroup_paths)
		tst_cgroup_paths = b->next;
	else
		a->next = b->next;

	free(b->mnt_path);
	free(b->new_path);
	free(b);
}

void tst_cgroup_mount(enum tst_cgroup_ctrl ctrl, const char *cgroup_dir)
{
	char *cgroup_new_dir;
	char knob_path[PATH_MAX];

	tst_cg_ver = tst_cgroup_version();

	tst_cgroup_set_path(cgroup_dir);
	cgroup_new_dir = tst_cgroup_get_path(cgroup_dir);

	if (tst_cg_ver & TST_CGROUP_V1) {
		switch(ctrl) {
		case TST_CGROUP_MEMCG:
			tst_cgroup1_mount("memcg", "memory", cgroup_dir, cgroup_new_dir);
		break;
		case TST_CGROUP_CPUSET:
			tst_cgroup1_mount("cpusetcg", "cpuset", cgroup_dir, cgroup_new_dir);
		break;
		default:
			tst_brk(TBROK, "Invalid cgroup controller: %d", ctrl);
		}
	}

	if (tst_cg_ver & TST_CGROUP_V2) {
		tst_cgroup2_mount(cgroup_dir, cgroup_new_dir);

		switch(ctrl) {
		case TST_CGROUP_MEMCG:
			sprintf(knob_path, "%s/cgroup.subtree_control", cgroup_dir);
			SAFE_FILE_PRINTF(knob_path, "%s", "+memory");
		break;
		case TST_CGROUP_CPUSET:
			tst_brk(TCONF, "Cgroup v2 hasn't achieve cpuset subsystem");
		break;
		default:
			tst_brk(TBROK, "Invalid cgroup controller: %d", ctrl);
		}
	}
}

void tst_cgroup_umount(const char *cgroup_dir)
{
	char *cgroup_new_dir;

	cgroup_new_dir = tst_cgroup_get_path(cgroup_dir);
	tst_cgroupN_umount(cgroup_dir, cgroup_new_dir);
	tst_cgroup_del_path(cgroup_dir);
}

void tst_cgroup_set_knob(const char *cgroup_dir, const char *knob, long value)
{
	char *cgroup_new_dir;
	char knob_path[PATH_MAX];

	cgroup_new_dir = tst_cgroup_get_path(cgroup_dir);
	sprintf(knob_path, "%s/%s", cgroup_new_dir, knob);
	SAFE_FILE_PRINTF(knob_path, "%ld", value);
}

void tst_cgroup_move_current(const char *cgroup_dir)
{
	if (tst_cg_ver & TST_CGROUP_V1)
		tst_cgroup_set_knob(cgroup_dir, "tasks", getpid());

	if (tst_cg_ver & TST_CGROUP_V2)
		tst_cgroup_set_knob(cgroup_dir, "cgroup.procs", getpid());
}

void tst_cgroup_mem_set_maxbytes(const char *cgroup_dir, long memsz)
{
	if (tst_cg_ver & TST_CGROUP_V1)
		tst_cgroup_set_knob(cgroup_dir, "memory.limit_in_bytes", memsz);

	if (tst_cg_ver & TST_CGROUP_V2)
		tst_cgroup_set_knob(cgroup_dir, "memory.max", memsz);
}

int tst_cgroup_mem_swapacct_enabled(const char *cgroup_dir)
{
	char *cgroup_new_dir;
	char knob_path[PATH_MAX];

	cgroup_new_dir = tst_cgroup_get_path(cgroup_dir);

	if (tst_cg_ver & TST_CGROUP_V1) {
		sprintf(knob_path, "%s/%s",
				cgroup_new_dir, "/memory.memsw.limit_in_bytes");

		if ((access(knob_path, F_OK) == -1)) {
			if (errno == ENOENT)
				tst_res(TCONF, "memcg swap accounting is disabled");
			else
				tst_brk(TBROK | TERRNO, "failed to access %s", knob_path);
		} else {
			return 1;
		}
	}

	if (tst_cg_ver & TST_CGROUP_V2) {
		sprintf(knob_path, "%s/%s",
				cgroup_new_dir, "/memory.swap.max");

		if ((access(knob_path, F_OK) == -1)) {
			if (errno == ENOENT)
				tst_res(TCONF, "memcg swap accounting is disabled");
			else
				tst_brk(TBROK | TERRNO, "failed to access %s", knob_path);
		} else {
			return 1;
		}
	}

	return 0;
}

void tst_cgroup_mem_set_maxswap(const char *cgroup_dir, long memsz)
{
	if (tst_cg_ver & TST_CGROUP_V1)
		tst_cgroup_set_knob(cgroup_dir, "memory.memsw.limit_in_bytes", memsz);

	if (tst_cg_ver & TST_CGROUP_V2)
		tst_cgroup_set_knob(cgroup_dir, "memory.swap.max", memsz);
}

void tst_cgroup_cpuset_read_files(const char *cgroup_dir, const char *filename,
	char *retbuf, size_t retbuf_sz)
{
	int fd;
	char *cgroup_new_dir;
	char knob_path[PATH_MAX];

	cgroup_new_dir = tst_cgroup_get_path(cgroup_dir);

	/*
	 * try either '/dev/cpuset/XXXX' or '/dev/cpuset/cpuset.XXXX'
	 * please see Documentation/cgroups/cpusets.txt from kernel src
	 * for details
	 */
	sprintf(knob_path, "%s/%s", cgroup_new_dir, filename);
	fd = open(knob_path, O_RDONLY);
	if (fd == -1) {
		if (errno == ENOENT) {
			sprintf(knob_path, "%s/cpuset.%s",
					cgroup_new_dir, filename);
			fd = SAFE_OPEN(knob_path, O_RDONLY);
		} else
			tst_brk(TBROK | TERRNO, "open %s", knob_path);
	}

	memset(retbuf, 0, retbuf_sz);
	if (read(fd, retbuf, retbuf_sz) < 0)
		tst_brk(TBROK | TERRNO, "read %s", knob_path);

	close(fd);
}

void tst_cgroup_cpuset_write_files(const char *cgroup_dir, const char *filename, const char *buf)
{
	int fd;
	char *cgroup_new_dir;
	char knob_path[PATH_MAX];

	cgroup_new_dir = tst_cgroup_get_path(cgroup_dir);

	/*
	 * try either '/dev/cpuset/XXXX' or '/dev/cpuset/cpuset.XXXX'
	 * please see Documentation/cgroups/cpusets.txt from kernel src
	 * for details
	 */
	sprintf(knob_path, "%s/%s", cgroup_new_dir, filename);
	fd = open(knob_path, O_WRONLY);
	if (fd == -1) {
		if (errno == ENOENT) {
			sprintf(knob_path, "%s/cpuset.%s", cgroup_new_dir, filename);
			fd = SAFE_OPEN(knob_path, O_WRONLY);
		} else
			tst_brk(TBROK | TERRNO, "open %s", knob_path);
	}

	SAFE_WRITE(1, fd, buf, strlen(buf));

	close(fd);
}
