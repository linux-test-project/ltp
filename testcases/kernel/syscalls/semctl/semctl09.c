// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Feiyu Zhu <zhufy.jy@cn.fujitsu.com>
 */
/*\
 * Call semctl() with SEM_INFO flag and check that:
 *
 * * The returned index points to a valid SEM by calling SEM_STAT_ANY
 * * Also count that valid indexes < returned max index sums up to semusz
 * * And the data are consistent with /proc/sysvipc/sem
 *
 * There is a possible race between the call to the semctl() and read from the
 * proc file so this test cannot be run in parallel with any IPC testcases that
 * adds or removes semaphore set.
 *
 * Note what we create a semaphore set in the test setup to make sure
 * that there is at least one during the testrun.
 *
 * Also note that for SEM_INFO the members of the seminfo structure have
 * completely different meaning than their names seems to suggest.
 *
 * We also calling semctl() directly by syscall(), because of a glibc bug:
 *
 * semctl SEM_STAT_ANY fails to pass the buffer specified by the caller
 * to the kernel.
 *
 * https://sourceware.org/bugzilla/show_bug.cgi?id=26637
 */

/*
 * The glibc bug was fixed in:
 *
 * * commit  574500a108be1d2a6a0dc97a075c9e0a98371aba
 * * Author: Dmitry V. Levin <ldv@altlinux.org>
 * * Date:   Tue, 29 Sep 2020 17:10:20 +0000 (14:10 -0300)
 */

#include <stdio.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"
#include "lapi/sem.h"
#include "lapi/syscalls.h"

static int sem_id = -1;
static uid_t nobody_uid, root_uid;
static union semun un;

/*
 * Note: semctl man-pages may have wrong description. We should use sem_ds
 * struct(un.buf) instead of seminfo struct(un.__buf).
 */
static inline int do_semctl(int semid, int semnum, int cmd)
{
	struct semid_ds info;

	un.buf = &info;

	switch (tst_variant) {
	case 0:
		return tst_syscall(__NR_semctl, semid, semnum, cmd, un);
	case 1:
		return semctl(semid, semnum, cmd, un);
	}
	return -1;
}

static void test_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Test SYS_semctl syscall");
	break;
	case 1:
		tst_res(TINFO, "Test libc semctl()");
	break;
	}
}

static struct tcases {
	uid_t *uid;
	char *desc;
} tests[] = {
	{&nobody_uid, "with nobody user",},
	{&root_uid, "with root user",},
};

static void parse_proc_sysvipc(struct seminfo *info)
{
	FILE *f = fopen("/proc/sysvipc/sem", "r");
	int semset_cnt = 0;
	int sem_cnt = 0;

	/* Eat header */
	for (;;) {
		int c = fgetc(f);

		if (c == '\n' || c == EOF)
			break;
	}

	int nsems;
	/*
	 * Sum sem set, nsems for all elements listed, which should equal
	 * the data returned in the seminfo structure.
	 */
	while (fscanf(f, "%*i %*i %*i %i %*i %*i %*i %*i %*i %*i",
		      &nsems) > 0){
		semset_cnt++;
		sem_cnt += nsems;
	}

	if (info->semusz != semset_cnt) {
		tst_res(TFAIL, "semusz = %i, expected %i",
				info->semusz, semset_cnt);
	} else {
		tst_res(TPASS, "semset_cnt = %i", semset_cnt);
	}

	if (info->semaem != sem_cnt) {
		tst_res(TFAIL, "semaem = %i, expected %i",
				info->semaem, sem_cnt);
	} else {
		tst_res(TPASS, "sen_cnt = %i", sem_cnt);
	}

	fclose(f);
}

static void verify_semctl(unsigned int n)
{
	struct tcases *tc = &tests[n];
	int i, semid, cnt = 0;
	struct seminfo info;
	union semun arg;

	tst_res(TINFO, "Test SEM_STAT_ANY %s", tc->desc);

	SAFE_SETEUID(*tc->uid);

	arg.__buf = &info;

	TEST(semctl(sem_id, 0, SEM_INFO, arg));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "semctl(sem_id, 0, SEM_INFO, ...)");
		return;
	}

	semid = do_semctl(TST_RET, 0, SEM_STAT_ANY);

	if (errno == EFAULT) {
		tst_res(TFAIL, "SEM_STAT_ANY doesn't pass the buffer "
				"specified by the caller to kernel");
		return;
	} else if (semid == -1) {
		tst_res(TFAIL | TERRNO, "SEM_INFO haven't returned a valid index");
	} else {
		tst_res(TPASS, "SEM_INFO returned valid index %li to semid %i",
			TST_RET, semid);
	}

	for (i = 0; i <= TST_RET; i++) {
		if ((do_semctl(i, 0, SEM_STAT_ANY)) != -1)
			cnt++;
	}

	if (cnt == info.semusz) {
		tst_res(TPASS, "Counted used = %i", cnt);
	} else {
		tst_res(TFAIL, "Counted used = %i, semuse = %i",
			cnt, info.semusz);
	}

	parse_proc_sysvipc(&info);
}

static void setup(void)
{
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");

	nobody_uid = ltpuser->pw_uid;
	root_uid = 0;
	test_info();

#if !HAVE_DECL_SEM_STAT_ANY
	if (tst_variant == 1)
		tst_brk(TCONF, "libc does not support semctl(SEM_STAT_ANY)");
#endif

	sem_id = SAFE_SEMGET(IPC_PRIVATE, 2, IPC_CREAT | 0600);

	TEST(do_semctl(sem_id, 0, SEM_STAT_ANY));
	if (TST_RET == -1) {
		if (TST_ERR == EFAULT)
			tst_brk(TFAIL,
				"SEM_STAT_ANY doesn't pass the buffer specified by the caller to kernel");
		if (TST_ERR == EINVAL)
			tst_brk(TCONF, "kernel doesn't support SEM_STAT_ANY");
		else
			tst_brk(TBROK | TTERRNO,
				"Current environment doesn't permit SEM_STAT_ANY");
	}
}

static void cleanup(void)
{
	SAFE_SETEUID(root_uid);

	if (sem_id >= 0)
		SAFE_SEMCTL(sem_id, 0, IPC_RMID);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_semctl,
	.tcnt = ARRAY_SIZE(tests),
	.test_variants = 2,
	.needs_root = 1,
	.tags = (const struct tst_tag[]) {
		{"glibc-git", "574500a108be"},
		{}
	}
};
