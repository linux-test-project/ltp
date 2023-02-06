// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email : code@zilogic.com
 */

/*\
 * [Description]
 *
 * This code tests the following flags:
 *
 * - AT_STATX_FORCE_SYNC
 * - AT_STATX_DONT_SYNC
 *
 * By exportfs cmd creating NFS setup.
 *
 * A test file is created in server folder and statx is being
 * done in client folder.
 *
 * BY AT_STATX_SYNC_AS_STAT getting predefined mode value.
 * Then, by using AT_STATX_FORCE_SYNC getting new updated vaue
 * from server file changes.
 *
 * BY AT_STATX_SYNC_AS_STAT getting predefined mode value.
 * AT_STATX_FORCE_SYNC is called to create cache data of the file.
 * Then, by using DONT_SYNC_FILE getting old cached data in client folder,
 * but mode has been chaged in server file.
 *
 * The support for SYNC flags was implemented in NFS in:
 * 9ccee940bd5b ("Support statx() mask and query flags parameters")
 */

#define _GNU_SOURCE
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/mount.h>
#include "tst_test.h"
#include "lapi/stat.h"

#define MODE(X) (X & (~S_IFMT))
#define FLAG_NAME(x) .flag = x, .flag_name = #x
#define BUFF_SIZE PATH_MAX
#define DEFAULT_MODE 0644
#define CURRENT_MODE 0777

#define CLI_PATH "client"
#define SERV_PATH "server"
#define CLI_FORCE_SYNC "client/force_sync_file"
#define CLI_DONT_SYNC "client/dont_sync_file"
#define SERV_FORCE_SYNC "server/force_sync_file"
#define SERV_DONT_SYNC "server/dont_sync_file"

static char *cwd;
static char cmd[BUFF_SIZE];
static int mounted;
static int exported;

static int get_mode(char *file_name, int flag_type, char *flag_name)
{
	struct statx buf;

	TEST(statx(AT_FDCWD, file_name, flag_type, STATX_ALL, &buf));

	if (TST_RET == -1) {
		tst_brk(TFAIL | TST_ERR,
			"statx(AT_FDCWD, %s, %s, STATX_ALL, &buf)",
			file_name, flag_name);
	}

	tst_res(TINFO, "statx(AT_FDCWD, %s, %s, STATX_ALL, &buf) = %o",
		file_name, flag_name, buf.stx_mode);

	return buf.stx_mode;
}

static const struct test_cases {
	int flag;
	char *flag_name;
	char *server_file;
	char *client_file;
	unsigned int mode;
} tcases[] = {
	{FLAG_NAME(AT_STATX_DONT_SYNC), SERV_DONT_SYNC, CLI_DONT_SYNC, DEFAULT_MODE},
	{FLAG_NAME(AT_STATX_FORCE_SYNC), SERV_FORCE_SYNC, CLI_FORCE_SYNC, CURRENT_MODE}
};

static void test_statx(unsigned int i)
{
	const struct test_cases *tc = &tcases[i];
	unsigned int cur_mode;

	get_mode(tc->client_file, AT_STATX_FORCE_SYNC, "AT_STATX_FORCE_SYNC");

	SAFE_CHMOD(tc->server_file, CURRENT_MODE);
	cur_mode = get_mode(tc->client_file, tc->flag, tc->flag_name);

	if (MODE(cur_mode) == tc->mode) {
		tst_res(TPASS,
			"statx() with %s for mode %o",
			tc->flag_name, tc->mode);
	} else {
		tst_res(TFAIL,
			"statx() with %s for mode %o %o",
			tc->flag_name, tc->mode, MODE(cur_mode));
	}

	SAFE_CHMOD(tc->server_file, DEFAULT_MODE);
}

static void setup(void)
{
	int ret;
	char server_path[BUFF_SIZE];

	cwd = tst_get_tmpdir();

	SAFE_MKDIR(SERV_PATH, DEFAULT_MODE);
	SAFE_MKDIR(CLI_PATH, DEFAULT_MODE);
	SAFE_CREAT(SERV_FORCE_SYNC, DEFAULT_MODE);
	SAFE_CREAT(SERV_DONT_SYNC, DEFAULT_MODE);

	snprintf(server_path, sizeof(server_path), ":%s/%s", cwd, SERV_PATH);

	snprintf(cmd, sizeof(cmd),
		 "exportfs -i -o no_root_squash,rw,sync,no_subtree_check,fsid=%d *%.1024s",
		 getpid(), server_path);
	exported = 1;

	ret = tst_system(cmd);
	if (ret)
		tst_brk(TBROK | TST_ERR, "failed to exportfs");

	if (mount(server_path, CLI_PATH, "nfs", 0, "addr=127.0.0.1")) {
		if (errno == EOPNOTSUPP || errno == ECONNREFUSED
			|| errno == ETIMEDOUT)
			tst_brk(TCONF | TERRNO, "nfs server not set up?");
		tst_brk(TBROK | TERRNO, "mount() nfs failed");
	}
	mounted = 1;
}

static void cleanup(void)
{
	if (mounted)
		SAFE_UMOUNT(CLI_PATH);

	if (!exported)
		return;
	snprintf(cmd, sizeof(cmd),
		 "exportfs -u *:%s/%s", cwd, SERV_PATH);

	if (tst_system(cmd) == -1)
		tst_res(TWARN | TST_ERR, "failed to clear exportfs");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = test_statx,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "4.16",
	.needs_tmpdir = 1,
	.dev_fs_type = "nfs",
	.needs_root = 1,
	.needs_cmds = (const char *[]) {
		"exportfs",
		NULL
	}
};
