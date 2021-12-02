// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * [Description]
 *
 * Call shmctl() with SHM_INFO flag and check that:
 *
 * * The returned index points to a valid SHM by calling SHM_STAT_ANY
 * * Also count that valid indexes < returned max index sums up to used_ids
 * * And the data are consistent with /proc/sysvipc/shm
 *
 * There is a possible race between the call to the shmctl() and read from the
 * proc file so this test cannot be run in parallel with any IPC testcases that
 * adds or removes SHM segments.
 *
 * Note what we create a SHM segment in the test setup to make sure that there
 * is at least one during the testrun.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <pwd.h>
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "libnewipc.h"
#include "lapi/shm.h"

#define SHM_SIZE 2048

static int shm_id = -1;
static uid_t nobody_uid, root_uid;

static struct tcases {
	uid_t *uid;
	char *desc;
} tests[] = {
	{&nobody_uid, "with nobody user"},
	{&root_uid, "with root user"}
};

static void parse_proc_sysvipc(struct shm_info *info)
{
	int page_size = getpagesize();
	FILE *f = fopen("/proc/sysvipc/shm", "r");
	int used_ids = 0;
	int shmid_max = 0;
	unsigned long shm_rss = 0;
	unsigned long shm_swp = 0;
	unsigned long shm_tot = 0;

	/* Eat header */
	for (;;) {
		int c = fgetc(f);

		if (c == '\n' || c == EOF)
			break;
	}

	int shmid, size, rss, swap;

	/*
	 * Sum rss, swap and size for all elements listed, which should equal
	 * the data returned in the shm_info structure.
	 *
	 * Note that the size has to be rounded up to nearest multiple of page
	 * size.
	 */
	while (fscanf(f, "%*i %i %*i %i %*i %*i %*i %*i %*i %*i %*i %*i %*i %*i %i %i",
			&shmid, &size, &rss, &swap) > 0) {
		used_ids++;
		shm_rss += rss/page_size;
		shm_swp += swap/page_size;
		shm_tot += (size + page_size - 1) / page_size;
		if (shmid > shmid_max)
			shmid_max = shmid;
	}

	if (info->used_ids != used_ids) {
		tst_res(TFAIL, "used_ids = %i, expected %i",
			info->used_ids, used_ids);
	} else {
		tst_res(TPASS, "used_ids = %i", used_ids);
	}

	if (info->shm_rss != shm_rss) {
		tst_res(TFAIL, "shm_rss = %li, expected %li",
			info->shm_rss, shm_rss);
	} else {
		tst_res(TPASS, "shm_rss = %li", shm_rss);
	}

	if (info->shm_swp != shm_swp) {
		tst_res(TFAIL, "shm_swp = %li, expected %li",
			info->shm_swp, shm_swp);
	} else {
		tst_res(TPASS, "shm_swp = %li", shm_swp);
	}

	if (info->shm_tot != shm_tot) {
		tst_res(TFAIL, "shm_tot = %li, expected %li",
			info->shm_tot, shm_tot);
	} else {
		tst_res(TPASS, "shm_tot = %li", shm_tot);
	}

	fclose(f);
}

static void verify_shminfo(unsigned int n)
{
	struct tcases *tc = &tests[n];
	struct shm_info info;
	struct shmid_ds ds;
	int i, shmid, cnt = 0;

	tst_res(TINFO, "Test SHM_STAT_ANY %s", tc->desc);

	SAFE_SETEUID(*tc->uid);

	TEST(shmctl(0, SHM_INFO, (struct shmid_ds *)&info));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "shmctl(0, SHM_INFO, ...)");
		return;
	}

	shmid = shmctl(TST_RET, SHM_STAT_ANY, &ds);

	if (shmid == -1) {
		tst_res(TFAIL | TERRNO, "SHM_INFO haven't returned a valid index");
	} else {
		tst_res(TPASS,
			"SHM_INFO returned valid index %li maps to shmid %i",
			TST_RET, shmid);
	}

	for (i = 0; i <= TST_RET; i++) {
		if (shmctl(i, SHM_STAT_ANY, &ds) != -1)
			cnt++;
	}

	if (cnt == info.used_ids) {
		tst_res(TPASS, "Counted used = %i", cnt);
	} else {
		tst_res(TFAIL, "Counted used = %i, used_ids = %i",
			cnt, info.used_ids);
	}

	parse_proc_sysvipc(&info);
}

static void setup(void)
{
	struct passwd *ltpuser = SAFE_GETPWNAM("nobody");
	struct shmid_ds temp_ds;

	nobody_uid = ltpuser->pw_uid;
	root_uid = 0;

	shm_id = SAFE_SHMGET(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | SHM_RW);

	TEST(shmctl(shm_id, SHM_STAT_ANY, &temp_ds));
	if (TST_RET == -1) {
		if (TST_ERR == EINVAL)
			tst_brk(TCONF, "kernel doesn't support SHM_STAT_ANY");
		else
			tst_brk(TBROK | TTERRNO,
				"Current environment doesn't permit SHM_STAT_ANY");
	}
}

static void cleanup(void)
{
	if (shm_id >= 0)
		SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_shminfo,
	.tcnt = ARRAY_SIZE(tests),
	.needs_root = 1,
};
