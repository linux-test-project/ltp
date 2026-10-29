// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test the format parsing of ``KEYCTL_DESCRIBE`` of :manpage:`keyctl(2)`.
 *
 * ``KEYCTL_DESCRIBE`` formats a key description as
 * "type;uid;gid;perm;description" and returns the full length of the
 * string including the terminating NUL byte.
 *
 * [Algorithm]
 *
 * - describe a valid key and verify all five fields of the description
 *   match the key's attributes
 */

#include <stdio.h>
#include <unistd.h>

#include "keyctl_common.h"

#define KEY_DESC	"ltpkeyctl10"
#define PAYLOAD		"payload"
#define BUF_SIZE	64

static key_serial_t key;
static long desc_len;
static char buf[BUF_SIZE];

static void setup(void)
{
	SAFE_KEYCTL(KEYCTL_JOIN_SESSION_KEYRING, 0, 0, 0, 0);

	key = SAFE_NEW_USER_KEY(KEY_DESC, PAYLOAD, sizeof(PAYLOAD),
			   KEY_SPEC_PROCESS_KEYRING);
	SAFE_KEYCTL(KEYCTL_SETPERM, key, KEY_PERM_SET, 0, 0);

	TEST(keyctl(KEYCTL_DESCRIBE, key, (unsigned long)NULL, 0, 0));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "KEYCTL_DESCRIBE failed");

	desc_len = TST_RET;
}

static void run(void)
{
	char type[32], desc[BUF_SIZE];
	unsigned int uid, gid, perm;

	memset(buf, 0, sizeof(buf));
	TST_EXP_EQ_LI_SILENT(keyctl(KEYCTL_DESCRIBE, key, (unsigned long)buf,
				    sizeof(buf), 0), desc_len);
	if (!TST_PASS)
		return;

	TST_EXP_EQ_SZ_SILENT((size_t)desc_len, strlen(buf) + 1);
	if (!TST_PASS) {
		tst_res(TINFO, "unexpected description '%s'", buf);
		return;
	}

	if (sscanf(buf, "%31[^;];%u;%u;%x;%63[^\n]",
		   type, &uid, &gid, &perm, desc) != 5) {
		tst_res(TFAIL, "malformed description '%s'", buf);
		return;
	}

	if (strcmp(type, "user")) {
		tst_res(TFAIL, "wrong key type '%s', expected 'user'", type);
		return;
	}

	if (uid != getuid() || gid != getgid()) {
		tst_res(TFAIL, "wrong owner uid %u gid %u, expected %u %u",
			uid, gid, getuid(), getgid());
		return;
	}

	if (perm != KEY_PERM_SET) {
		tst_res(TFAIL, "wrong permissions %08x, expected %08x",
			perm, KEY_PERM_SET);
		return;
	}

	if (strcmp(desc, KEY_DESC)) {
		tst_res(TFAIL, "wrong description '%s', expected '%s'",
			desc, KEY_DESC);
		return;
	}

	tst_res(TPASS, "described the key correctly");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
};
