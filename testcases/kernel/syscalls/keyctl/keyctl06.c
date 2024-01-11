// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Google, Inc.
 * Copyright (c) Linux Test Project, 2017-2024
 */

/*\
 * [Description]
 *
 * Regression test for commit:
 *
 * e645016abc80 ("KEYS: fix writing past end of user-supplied buffer in keyring_read()")
 *
 * as well as its follow-on fix:
 *
 * commit 3239b6f29bdf ("KEYS: return full count in keyring_read() if buffer is too small")
 */

#include <errno.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

static void add_test_key(const char *description)
{
	TEST(add_key("user", description, "payload", 7,
		     KEY_SPEC_PROCESS_KEYRING));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "Failed to add test key");
}

static void do_test(void)
{
	key_serial_t key_ids[2];

	add_test_key("key1");
	add_test_key("key2");

	memset(key_ids, 0, sizeof(key_ids));
	TEST(keyctl(KEYCTL_READ, KEY_SPEC_PROCESS_KEYRING,
		    (char *)key_ids, sizeof(key_serial_t)));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "KEYCTL_READ failed");

	/*
	 * Do not check key_ids[0], as the contents of the buffer are
	 * unspecified if it was too small.  However, key_ids[1] must not have
	 * been written to, as it was outside the buffer.
	 */

	if (key_ids[1] != 0)
		tst_brk(TFAIL, "KEYCTL_READ overran the buffer");

	if (TST_RET != sizeof(key_ids)) {
		tst_brk(TFAIL, "KEYCTL_READ returned %ld but expected %zu",
			TST_RET, sizeof(key_ids));
	}

	tst_res(TPASS,
		"KEYCTL_READ returned full count but didn't overrun the buffer");
}

static struct tst_test test = {
	.test_all = do_test,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "e645016abc80"},
		{"linux-git", "3239b6f29bdf"},
		{}
	}
};
