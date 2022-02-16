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
 * Test confstr(3) 500 (X/Open 5) functionality &ndash; POSIX 1995.
 */

#include "tst_test.h"

#ifdef _XOPEN_SOURCE
# undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <unistd.h>

static struct test_case_t {
	int value;
	char *name;
} test_cases[] = {
	{ _CS_PATH, "PATH" },
	{ _CS_GNU_LIBC_VERSION, "GNU_LIBC_VERSION" },
	{ _CS_GNU_LIBPTHREAD_VERSION, "GNU_LIBPTHREAD_VERSION" },

/* checking for obsolete configurations */
#ifdef _CS_XBS5_ILP32_OFF32_CFLAGS
	{ _CS_XBS5_ILP32_OFF32_CFLAGS, "XBS5_ILP32_OFF32_CFLAGS" },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LDFLAGS
	{ _CS_XBS5_ILP32_OFF32_LDFLAGS, "XBS5_ILP32_OFF32_LDFLAGS" },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LIBS
	{ _CS_XBS5_ILP32_OFF32_LIBS, "XBS5_ILP32_OFF32_LIBS" },
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LINTFLAGS
	{ _CS_XBS5_ILP32_OFF32_LINTFLAGS, "XBS5_ILP32_OFF32_LINTFLAGS" },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_CFLAGS
	{ _CS_XBS5_ILP32_OFFBIG_CFLAGS, "XBS5_ILP32_OFFBIG_CFLAGS" },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LDFLAGS
	{ _CS_XBS5_ILP32_OFFBIG_LDFLAGS, "XBS5_ILP32_OFFBIG_LDFLAGS" },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LIBS
	{ _CS_XBS5_ILP32_OFFBIG_LIBS, "XBS5_ILP32_OFFBIG_LIBS" },
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LINTFLAGS
	{ _CS_XBS5_ILP32_OFFBIG_LINTFLAGS, "XBS5_ILP32_OFFBIG_LINTFLAGS" },
#endif
#ifdef _CS_XBS5_LP64_OFF64_CFLAGS
	{ _CS_XBS5_LP64_OFF64_CFLAGS, "XBS5_LP64_OFF64_CFLAGS" },
#endif
#ifdef _CS_XBS5_LP64_OFF64_LDFLAGS
	{ _CS_XBS5_LP64_OFF64_LDFLAGS, "XBS5_LP64_OFF64_LDFLAGS" },
#endif
#ifdef _CS_XBS5_LP64_OFF64_LIBS
	{ _CS_XBS5_LP64_OFF64_LIBS, "XBS5_LP64_OFF64_LIBS" },
#endif
#ifdef _CS_XBS5_LP64_OFF64_LINTFLAGS
	{ _CS_XBS5_LP64_OFF64_LINTFLAGS, "XBS5_LP64_OFF64_LINTFLAGS" },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_CFLAGS
	{ _CS_XBS5_LPBIG_OFFBIG_CFLAGS, "XBS5_LPBIG_OFFBIG_CFLAGS" },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LDFLAGS
	{ _CS_XBS5_LPBIG_OFFBIG_LDFLAGS, "XBS5_LPBIG_OFFBIG_LDFLAGS" },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LIBS
	{ _CS_XBS5_LPBIG_OFFBIG_LIBS, "XBS5_LPBIG_OFFBIG_LIBS" },
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS
	{ _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS, "XBS5_LPBIG_OFFBIG_LINTFLAGS" },
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
