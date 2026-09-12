// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2007 David Gibson & Adam Litke, IBM Corporation.
 * Copyright (c) Linux Test Project, 2024
 * Copyright (C) 2025-2026 Naveed & Pavithra, IBM Corporation.
 * Assisted with AI tools
 */

/*\
 * Test hugetlbfs quota accounting with filesystem size limits.
 *
 * The number of global huge pages available to a mounted hugetlbfs filesystem
 * can be limited using a quota mechanism by setting the size attribute at
 * mount time. Older kernels did not properly handle quota accounting for
 * MAP_PRIVATE pages and MAP_SHARED reservations.
 *
 * Commit a1e78772d72b introduced MAP_PRIVATE reservations at
 * :manpage:`mmap(2)` time, so quota is checked upfront and
 * :manpage:`mmap(2)` fails with ENOMEM when over quota.
 * Before this, quota was only checked at fault time resulting in SIGBUS.
 *
 * Root is required to mount the quota-limited hugetlbfs instance.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/vfs.h>
#include <sys/mount.h>
#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"

static long hpage_size;
static int quota_mounted;

static void verify_quota_stat(long total, long free, long avail)
{
	struct statfs s;

	SAFE_STATFS(MNTPOINT, &s);

	TST_EXP_EQ_LI(s.f_blocks, total);
	TST_EXP_EQ_LI(s.f_bfree, free);
	TST_EXP_EQ_LI(s.f_bavail, avail);
}

static void verify_quota_map_unmap(unsigned long size, int mmap_flags)
{
	int fd = tst_creat_unlinked(MNTPOINT, 0, 0600);

	TST_EXP_PASS_PTR_VOID(mmap(NULL, size, PROT_READ | PROT_WRITE, mmap_flags, fd, 0),
			       "mmap untouched mapping");
	if (TST_PASS)
		SAFE_MUNMAP(TST_RET_PTR, size);
	SAFE_CLOSE(fd);
}

static void verify_quota_ok(unsigned long size, int mmap_flags)
{
	int fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	char *b;

	TST_EXP_PASS_PTR_VOID(mmap(NULL, size, PROT_READ | PROT_WRITE, mmap_flags, fd, 0),
			       "mmap within quota");
	if (TST_PASS) {
		for (b = TST_RET_PTR; b < (char *)TST_RET_PTR + size; b += hpage_size)
			*b = 1;
		SAFE_MUNMAP(TST_RET_PTR, size);
	}
	SAFE_CLOSE(fd);
}

static void verify_quota_fail(unsigned long size, int mmap_flags)
{
	int fd = tst_creat_unlinked(MNTPOINT, 0, 0600);

	TST_EXP_FAIL_PTR_VOID(mmap(NULL, size, PROT_READ | PROT_WRITE, mmap_flags, fd, 0),
			       ENOMEM, "mmap over quota");
	if (TST_RET_PTR != MAP_FAILED)
		SAFE_MUNMAP(TST_RET_PTR, size);
	SAFE_CLOSE(fd);
}

static void verify_quota_private_resv_fail(unsigned long size, int mmap_flags)
{
	int fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	char *a, *b;

	TST_EXP_PASS_PTR_VOID(mmap(NULL, size, PROT_READ | PROT_WRITE, mmap_flags, fd, 0),
			       "first mmap within quota");
	if (!TST_PASS) {
		SAFE_CLOSE(fd);
		return;
	}
	a = TST_RET_PTR;

	for (b = a; b < a + size; b += hpage_size)
		*b = 1;

	TST_EXP_FAIL_PTR_VOID(mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0),
			       ENOMEM, "MAP_PRIVATE mmap over quota");
	if (TST_RET_PTR != MAP_FAILED)
		SAFE_MUNMAP(TST_RET_PTR, size);
	SAFE_MUNMAP(a, size);
	SAFE_CLOSE(fd);
}

static struct tcase {
	void (*fn)(unsigned long size, int mmap_flags);
	unsigned long size_mult;	/* multiplied by hpage_size at runtime */
	int mmap_flags;
	const char *desc;
	int check_stat;			/* call verify_quota_stat(1,1,1) after */
} tcases[] = {
	{ verify_quota_map_unmap, 1, MAP_PRIVATE,
	  "Unused quota cleanup for untouched MAP_PRIVATE mapping", 1 },
	{ verify_quota_map_unmap, 1, MAP_SHARED,
	  "Unused quota cleanup for untouched MAP_SHARED mapping", 1 },
	{ verify_quota_ok, 1, MAP_PRIVATE,
	  "Page instantiation within quota (MAP_PRIVATE)", 0 },
	{ verify_quota_ok, 1, MAP_SHARED,
	  "Page instantiation within quota (MAP_SHARED)", 0 },
	{ verify_quota_fail, 2, MAP_SHARED,
	  "Page instantiation over quota (MAP_SHARED)", 0 },
	{ verify_quota_fail, 2, MAP_PRIVATE,
	  "Private mapping quota check (MAP_PRIVATE)", 0 },
	{ verify_quota_private_resv_fail, 1, MAP_SHARED,
	  "Second MAP_PRIVATE reservation over quota (MAP_SHARED base)", 0 },
	{ verify_quota_private_resv_fail, 1, MAP_PRIVATE,
	  "Second MAP_PRIVATE reservation over quota (MAP_PRIVATE base)", 0 },
	{ verify_quota_ok, 1, MAP_SHARED,
	  "Operation within quota after failures (MAP_SHARED)", 0 },
	{ verify_quota_ok, 1, MAP_PRIVATE,
	  "Operation within quota after failures (MAP_PRIVATE)", 0 },
};

static void run_test(unsigned int i)
{
	const struct tcase *tc = &tcases[i];
	pid_t pid;
	int status;

	tst_res(TINFO, "%s", tc->desc);

	pid = SAFE_FORK();
	if (pid == 0) {
		tc->fn(tc->size_mult * hpage_size, tc->mmap_flags);
		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);

	if (WIFSIGNALED(status))
		tst_res(TFAIL, "Child killed by signal %d", WTERMSIG(status));

	if (tc->check_stat)
		verify_quota_stat(1, 1, 1);
}

static void setup(void)
{
	char mount_opts[BUFSIZ];

	hpage_size = tst_get_hugepage_size();

	snprintf(mount_opts, sizeof(mount_opts), "size=%luK",
		 hpage_size / 1024);

	SAFE_MOUNT("none", MNTPOINT, "hugetlbfs", 0, mount_opts);
	quota_mounted = 1;

	tst_res(TINFO, "Mounted hugetlbfs with quota at %s (size=%luK)",
		MNTPOINT, hpage_size / 1024);
}

static void cleanup(void)
{
	if (quota_mounted)
		SAFE_UMOUNT(MNTPOINT);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "a1e78772d72b2616ed20e54896e68e0e7044854e"},
		{}
	},
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = run_test,
	.tcnt = ARRAY_SIZE(tcases),
	.hugepages = {2, TST_NEEDS},
};
