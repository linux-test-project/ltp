// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 *
 * Test unprivileged user can support the number of keys and the
 * number of bytes consumed in payloads of the keys. The default
 * value is 200 and 20000.
 *
 * This is also a regression test for
 * commit a08bf91ce28e ("KEYS: allow reaching the keys quotas exactly")
 * commit 2e356101e72a ("KEYS: reaching the keys quotas correctly")
 *
 * If you run this test with -i > 5 then expect to see some sporadic failures
 * where add_key fails with EDQUOT. Keys are freed asynchronously and we only
 * create up to 10 users to avoid race conditions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include "tst_test.h"
#include "lapi/keyctl.h"

#define MAX_USERS 10

static char *user_buf;
static uid_t ltpuser[MAX_USERS];

static unsigned int usern;
static unsigned int useri;

static void add_user(char n)
{
	char username[] = "ltp_add_key05_n";
	const char *const cmd_useradd[] = {"useradd", username, NULL};
	const char *const cmd_userdel[] = {"userdel", "-r", username, NULL};
	const char *const cmd_groupdel[] = {"groupdel", username, NULL};
	struct passwd *pw;

	username[sizeof(username) - 2] = '0' + n;

	tst_cmd(cmd_userdel, NULL, "/dev/null", TST_CMD_PASS_RETVAL);
	tst_cmd(cmd_groupdel, NULL, "/dev/null", TST_CMD_PASS_RETVAL);

	SAFE_CMD(cmd_useradd, NULL, NULL);
	pw = SAFE_GETPWNAM(username);
	ltpuser[(unsigned int)n] = pw->pw_uid;

	tst_res(TINFO, "Created user %s", pw->pw_name);
}

static void clean_user(char n)
{
	char username[] = "ltp_add_key05_n";
	const char *const cmd_userdel[] = {"userdel", "-r", username, NULL};

	username[sizeof(username) - 2] = '0' + n;

	if (tst_cmd(cmd_userdel, NULL, NULL, TST_CMD_PASS_RETVAL))
		tst_res(TWARN | TERRNO, "'userdel -r %s' failed", username);
}

static inline void parse_proc_key_users(int *used_key, int *max_key, int *used_bytes, int *max_bytes)
{
	unsigned int val[4];
	char fmt[1024];

	sprintf(fmt, "%5u: %%*5d %%*d/%%*d %%d/%%d %%d/%%d", ltpuser[useri]);
	SAFE_FILE_LINES_SCANF("/proc/key-users", fmt, &val[0], &val[1], &val[2], &val[3]);

	if (used_key)
		*used_key = val[0];
	if (max_key)
		*max_key = val[1];
	if (used_bytes)
		*used_bytes = val[2];
	if (max_bytes)
		*max_bytes = val[3];
}

static void verify_max_bytes(void)
{
	char *buf;
	int plen, invalid_plen, delta;
	int used_bytes, max_bytes, tmp_used_bytes;

	tst_res(TINFO, "test max bytes under unprivileged user");

	parse_proc_key_users(NULL, NULL, &tmp_used_bytes, NULL);
	TEST(add_key("user", "test2", user_buf, 64, KEY_SPEC_THREAD_KEYRING));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "add key test2 failed");
		return;
	}
	parse_proc_key_users(NULL, NULL, &used_bytes, &max_bytes);

	/*
	 * Compute delta between default datalen(in key_alloc) and actual
	 * datlen(in key_payload_reserve).
	 * more info see kernel code: security/keys/key.c
	 */
	delta = used_bytes - tmp_used_bytes - strlen("test2") - 1 - 64;
	invalid_plen = max_bytes - used_bytes - delta - strlen("test_xxx");
	buf = tst_alloc(invalid_plen);

	TEST(add_key("user", "test_inv", buf, invalid_plen, KEY_SPEC_THREAD_KEYRING));
	if (TST_RET != -1) {
		tst_res(TFAIL, "add_key(test_inv) succeeded unexpectedltly");
		return;
	}
	if (TST_ERR == EDQUOT)
		tst_res(TPASS | TTERRNO, "add_key(test_inv) failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "add_key(test_inv) failed expected EDQUOT got");

	/*Reset delta*/
	TEST(add_key("user", "test3", user_buf, 64, KEY_SPEC_THREAD_KEYRING));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "add key test3 failed");
		return;
	}
	TEST(add_key("user", "test4", user_buf, 64, KEY_SPEC_THREAD_KEYRING));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "add key test4 failed");
		return;
	}
	parse_proc_key_users(NULL, NULL, &used_bytes, &max_bytes);
	plen = max_bytes - used_bytes - delta - strlen("test_xxx") - 1;
	TEST(add_key("user", "test_max", buf, plen, KEY_SPEC_THREAD_KEYRING));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "add_key(test_max) failed unexpectedly");
		return;
	}

	tst_res(TPASS, "add_key(test_max) succeeded as expected");
	parse_proc_key_users(NULL, NULL, &tmp_used_bytes, &max_bytes);
	if (tmp_used_bytes == max_bytes)
		tst_res(TPASS, "allow reaching the max bytes exactly");
	else
		tst_res(TFAIL, "max used bytes %u, key allow max bytes %u", tmp_used_bytes, max_bytes);
}

static void verify_max_keys(void)
{
	int i, used_key, max_key;
	char desc[10];

	tst_res(TINFO, "test max keys under unprivileged user");
	parse_proc_key_users(&used_key, &max_key, NULL, NULL);

	for (i = used_key + 1; i <= max_key; i++) {
		sprintf(desc, "abc%d", i);
		TEST(add_key("user", desc, user_buf, 64, KEY_SPEC_THREAD_KEYRING));
		if (TST_RET == -1) {
			tst_res(TFAIL | TTERRNO, "add keyring key(%s) failed", desc);
			goto count;
		}
	}

	TEST(add_key("user", "test_invalid_key", user_buf, 64, KEY_SPEC_THREAD_KEYRING));
	if (TST_RET != -1) {
		tst_res(TFAIL, "add keyring key(test_invalid_key) succeeded unexpectedly");
		goto count;
	}
	if (TST_ERR == EDQUOT)
		tst_res(TPASS | TTERRNO, "add_key(test_invalid_key) failed as expected");
	else
		tst_res(TFAIL | TTERRNO, "add_key(test_invalid_key) failed expected EDQUOT got");

count:
	parse_proc_key_users(&used_key, &max_key, NULL, NULL);
	if (used_key == max_key)
		tst_res(TPASS, "allow reaching the max key(%u) exactly", max_key);
	else
		tst_res(TFAIL, "max used key %u, key allow max key %u", used_key, max_key);
}

static void do_test(unsigned int n)
{
	if (usern < MAX_USERS)
		add_user(usern++);

	if (useri >= MAX_USERS) {
		sleep(1);
		useri = 0;
	}

	if (!SAFE_FORK()) {
		SAFE_SETUID(ltpuser[useri]);
		tst_res(TINFO, "User: %d, UID: %d", useri, ltpuser[useri]);
		TEST(add_key("user", "test1", user_buf, 64, KEY_SPEC_THREAD_KEYRING));
		if (TST_RET == -1) {
			tst_res(TFAIL | TTERRNO, "add key test1 failed");
			return;
		}

		if (n)
			verify_max_keys();
		else
			verify_max_bytes();
	}

	tst_reap_children();
	useri++;

	return;
}

static void cleanup(void)
{
	while (usern--)
		clean_user(usern);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = 2,
	.needs_root = 1,
	.forks_child = 1,
	.cleanup = cleanup,
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/kernel/keys/gc_delay", "1",
			TST_SR_SKIP_MISSING | TST_SR_TCONF_RO},
		{"/proc/sys/kernel/keys/maxkeys", "200",
			TST_SR_SKIP_MISSING | TST_SR_TCONF_RO},
		{"/proc/sys/kernel/keys/maxbytes", "20000",
			TST_SR_SKIP_MISSING | TST_SR_TCONF_RO},
		{}
	},
	.bufs = (struct tst_buffers []) {
		{&user_buf, .size = 64},
		{}
	},
	.needs_cmds = (const char *const []) {
		"useradd",
		"userdel",
		"groupdel",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "a08bf91ce28"},
		{"linux-git", "2e356101e72"},
		{}
	}
};
