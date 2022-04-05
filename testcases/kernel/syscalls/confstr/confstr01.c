// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
*  11/20/2002 Port to LTP <robbiew@us.ibm.com>
*  06/30/2001 Port to Linux <nsharoff@us.ibm.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 * Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>
 */

/*\
 * [Description]
 *
 * Test confstr(3) 700 (X/Open 7) functionality &ndash; POSIX 2008.
 */

#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <unistd.h>

#include "tst_test.h"

#define PAIR(name) {_CS_ ## name, #name}

static struct test_case_t {
	int value;
	char *name;
} test_cases[] = {
	PAIR(PATH),
	PAIR(GNU_LIBC_VERSION),
	PAIR(GNU_LIBPTHREAD_VERSION),
	PAIR(POSIX_V7_ILP32_OFF32_CFLAGS),
	PAIR(POSIX_V7_ILP32_OFF32_LDFLAGS),
	PAIR(POSIX_V7_ILP32_OFF32_LIBS),
	PAIR(POSIX_V7_ILP32_OFFBIG_CFLAGS),
	PAIR(POSIX_V7_ILP32_OFFBIG_LDFLAGS),
	PAIR(POSIX_V7_ILP32_OFFBIG_LIBS),
	PAIR(POSIX_V7_LP64_OFF64_CFLAGS),
	PAIR(POSIX_V7_LP64_OFF64_LDFLAGS),
	PAIR(POSIX_V7_LP64_OFF64_LIBS),
	PAIR(POSIX_V7_LPBIG_OFFBIG_CFLAGS),
	PAIR(POSIX_V7_LPBIG_OFFBIG_LDFLAGS),
	PAIR(POSIX_V7_LPBIG_OFFBIG_LIBS),
#ifdef _CS_POSIX_V7_THREADS_CFLAGS
	PAIR(POSIX_V7_THREADS_CFLAGS),
#endif
#ifdef _CS_POSIX_V7_THREADS_LDFLAGS
	PAIR(POSIX_V7_THREADS_LDFLAGS),
#endif
	PAIR(POSIX_V7_WIDTH_RESTRICTED_ENVS),
#ifdef _CS_V7_ENV
	PAIR(V7_ENV),
#endif
};

static void run(unsigned int i)
{
	char *buf;
	int len;

	TST_EXP_POSITIVE(confstr(test_cases[i].value, NULL, (size_t)0));

	if (!TST_RET)
		return;

	len = TST_RET;
	buf = SAFE_MALLOC(len);

	TEST(confstr(test_cases[i].value, buf, len));

	if (buf[len - 1] != '\0') {
		tst_brk(TFAIL, "confstr: %s, %s", test_cases[i].name,
			tst_strerrno(TST_ERR));
	} else {
		tst_res(TPASS, "confstr %s = '%s'", test_cases[i].name, buf);
	}

	free(buf);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(test_cases),
};
