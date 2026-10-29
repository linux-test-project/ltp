// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 FUJITSU LIMITED. All Rights Reserved.
 * Author: Ma Xinjian <maxj.fnst@fujitsu.com>
 */

/*\
 * Verify that request_key(2) fails with
 *
 * - EFAULT when type points outside the process's accessible address space
 * - EFAULT when description points outside the process's accessible address space
 * - EFAULT when callout_info points outside the process's accessible address space
 * - EPERM when type argument started with a period '.'
 */

#include "tst_test.h"
#include "lapi/keyctl.h"

static struct test_case_t {
	char *type;
	char *description;
	char *callout_info;
	key_serial_t dest_keyring;
	int expected_errno;
	char *desc;
} tcases[] = {
	{(char *)(-1), "description", NULL, KEY_SPEC_PROCESS_KEYRING, EFAULT,
		"type points outside the process's accessible address space"},
	{"type", (char *)(-1), NULL, KEY_SPEC_PROCESS_KEYRING, EFAULT,
		"description points outside the process's accessible address space"},
	{"type", "description", (char *)(-1), KEY_SPEC_PROCESS_KEYRING, EFAULT,
		"callout_info points outside the process's accessible address space"},
	{".type", "description", NULL, KEY_SPEC_PROCESS_KEYRING, EPERM,
		"type argument started with a period '.'"},
};

static void verify_request_key(unsigned int i)
{
	struct test_case_t *tc = &tcases[i];

	TST_EXP_FAIL2(request_key(tc->type, tc->description, tc->callout_info,
		tc->dest_keyring),
		tc->expected_errno,
		"%s", tc->desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_request_key,
};
