// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 Google, Inc.
 * Copyright (c) Linux Test Project, 2023
 */

/*\
 * Test that encrypted keys can be instantiated using user-provided decrypted
 * data that is hex-ascii encoded.
 */

#include "tst_test.h"
#include "lapi/keyctl.h"

#define ENCRYPTED_KEY_VALID_PAYLOAD	"new enc32 user:masterkey 32 abcdefABCDEF1234567890aaaaaaaaaaabcdefABCDEF1234567890aaaaaaaaaa"
#define ENCRYPTED_KEY_INVALID_PAYLOAD	"new enc32 user:masterkey 32 plaintext123@123!123@123!123@123plaintext123@123!123@123!123@123"

static void do_test(void)
{
	char buffer[128];

	TST_EXP_POSITIVE(add_key("user", "user:masterkey", "foo", 3,
			    KEY_SPEC_PROCESS_KEYRING));

	if (!TST_PASS)
		return;

	TST_EXP_POSITIVE(add_key("encrypted", "ltptestkey1",
			    ENCRYPTED_KEY_VALID_PAYLOAD,
			    strlen(ENCRYPTED_KEY_VALID_PAYLOAD),
			    KEY_SPEC_PROCESS_KEYRING));

	if (!TST_PASS)
		return;

	TST_EXP_POSITIVE(keyctl(KEYCTL_READ, TST_RET, buffer, sizeof(buffer)));

	if (!TST_PASS)
		return;

	TST_EXP_FAIL2(add_key("encrypted", "ltptestkey2",
			    ENCRYPTED_KEY_INVALID_PAYLOAD,
			    strlen(ENCRYPTED_KEY_INVALID_PAYLOAD),
			    KEY_SPEC_PROCESS_KEYRING), EINVAL);

	keyctl(KEYCTL_CLEAR, KEY_SPEC_PROCESS_KEYRING);
}

static struct tst_test test = {
	.test_all = do_test,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_DECRYPTED_DATA=y",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{ "linux-git", "5adedd42245af"},
		{}
	}
};
