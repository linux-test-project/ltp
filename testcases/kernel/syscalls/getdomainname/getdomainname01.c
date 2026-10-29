// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *   AUTHOR: Saji Kumar.V.R <saji.kumar@wipro.com>
 */

/*\
 * Basic test for getdomainname(2)
 *
 * This is a Phase I test for the getdomainname(2) system call.
 * It is intended to provide a limited exposure of the system call.
 */

#include <linux/utsname.h>
#include "tst_test.h"

#define MAX_NAME_LEN __NEW_UTS_LEN

static void verify_getdomainname(void)
{
	char domain_name[MAX_NAME_LEN];

	TST_EXP_PASS(getdomainname(domain_name, sizeof(domain_name)));
}

static struct tst_test test = {
        .test_all = verify_getdomainname,
};
