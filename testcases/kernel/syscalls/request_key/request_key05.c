// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 * Copyright (c) Linux Test Project, 2018-2024
 */

/*\
 * Test for CVE-2017-6951, original reproducer:
 * http://www.spinics.net/lists/keyrings/msg01845.html
 *
 * request_key() is not in glibc, so we just use the syscall directly instead
 * of linking to keyutils.
 */

#include <unistd.h>
#include <sys/syscall.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

#define ATTEMPTS 0x100

static void run(void)
{
	int i;

	tst_res(TINFO, "Requesting dead key");
	for (i = 0; i < ATTEMPTS; i++)
		tst_syscall(__NR_request_key, "dead", "abc", "abc", 0, 0, 0);

	tst_res(TPASS, "No crash after %d attempts", ATTEMPTS);
}

static struct tst_test test = {
	.test_all = run,
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-6951"},
		{}
	}
};
