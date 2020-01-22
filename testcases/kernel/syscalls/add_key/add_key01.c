// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Porting from Crackerjack to LTP is done by
 * Manas Kumar Nayak maknayak@in.ibm.com>
 *
 * This case test various key type can support how many long
 * bytes payload.
 * keyring: 0 bytes
 * user/logon: 32767 bytes
 * big_key: 1M -1byte
 *
 * The tests needs root because larger keys are over limit for unpriviledged
 * user by default.
 */

#include "tst_test.h"
#include "lapi/keyctl.h"

static char *keyring_buf, *keyring_buf1;
static char *user_buf, *user_buf1;
static char *logon_buf, *logon_buf1;
static char *big_key_buf, *big_key_buf1;
static unsigned int logon_nsup, big_key_nsup;

struct tcase {
	const char *type;
	const char *desc;
	char **buf;
	size_t plen;
	int pass_flag;
	char *message;
} tcases[] = {
	{"keyring", "abc", &keyring_buf, 0, 1,
	"The key type is keyrings and plen is 0"},

	{"keyring", "bcd", &keyring_buf, 1, 0,
	"the key type is keyrings and plen is 1"},

	{"user", "cde", &user_buf, 32767, 1,
	"The key type is user and plen is 32767"},

	{"user", "def", &user_buf1, 32768, 0,
	"The key type is user and plen is 32768"},

	{"logon", "ef:g", &logon_buf, 32767, 1,
	"The key type is logon and plen is 32767"},

	{"logon", "fg:h", &logon_buf1, 32768, 0,
	"The key type is logon and plen is 32768"},

	{"big_key", "ghi", &big_key_buf, (1 << 20) - 1, 1,
	"The key type is big_key and plen is 1048575"},

	{"big_key", "hij", &big_key_buf1, 1 << 20, 0,
	"The key type is big_key and plen is 1048576"},
};

static void verify_add_key(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "%s", tc->message);

	if (!strcmp(tc->type, "logon") && logon_nsup) {
		tst_res(TCONF, "skipping unsupported logon key");
		return;
	}
	if (!strcmp(tc->type, "big_key") && big_key_nsup) {
		tst_res(TCONF, "skipping unsupported big_key key");
		return;
	}

	TEST(add_key(tc->type, tc->desc, *tc->buf, tc->plen, KEY_SPEC_THREAD_KEYRING));
	if (tc->pass_flag) {
		if (TST_RET == -1)
			tst_res(TFAIL | TTERRNO, "add_key call failed unexpectedly");
		else
			tst_res(TPASS, "add_key call succeeded as expected");
	} else {
		if (TST_RET == -1) {
			if (TST_ERR == EINVAL)
				tst_res(TPASS | TTERRNO, "add_key call failed as expected");
			else
				tst_res(TFAIL | TTERRNO, "add_key call failed expected EINVAL but got");
		} else {
			tst_res(TFAIL, "add_key call succeeded unexpectedly");
		}
	}
}

static void setup(void)
{
	char buf[64];

	TEST(add_key("logon", "test:sup_logon", buf, sizeof(buf), KEY_SPEC_THREAD_KEYRING));
	if (TST_RET == -1)
		logon_nsup = 1;

	TEST(add_key("big_key", "sup_big_key", buf, sizeof(buf), KEY_SPEC_THREAD_KEYRING));
	if (TST_RET == -1)
		big_key_nsup = 1;
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_add_key,
	.needs_root = 1,
	.bufs = (struct tst_buffers []) {
		{&keyring_buf, .size = 1},
		{&keyring_buf1, .size = 1},
		{&user_buf, .size = 32767},
		{&user_buf1, .size = 32768},
		{&logon_buf, .size = 32767},
		{&logon_buf1, .size = 32768},
		{&big_key_buf, .size = (1 << 20) - 1},
		{&big_key_buf1, .size = 1 << 20},
		{}
	}
};
