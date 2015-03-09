/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* 11/20/2002	Port to LTP	robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	confstr1.c - test for confstr(3C) - Get configuration-defined string
 *	values.
 *
 * CALLS
 *	confstr(3C)
 *
 * RESTRICTIONS
 * MUST RUN AS ROOT
 *
 */

#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>

#include "test.h"
#include "safe_macros.h"

static struct test_case_t {
	int value;
	char *name;
} test_cases[] = {
	{_CS_PATH, "PATH"},
	{_CS_XBS5_ILP32_OFF32_CFLAGS, "XBS5_ILP32_OFF32_CFLAGS"},
	{_CS_XBS5_ILP32_OFF32_LDFLAGS, "XBS5_ILP32_OFF32_LDFLAGS"},
	{_CS_XBS5_ILP32_OFF32_LIBS, "XBS5_ILP32_OFF32_LIBS"},
	{_CS_XBS5_ILP32_OFF32_LINTFLAGS, "XBS5_ILP32_OFF32_LINTFLAGS"},
	{_CS_XBS5_ILP32_OFFBIG_CFLAGS, "XBS5_ILP32_OFFBIG_CFLAGS"},
	{_CS_XBS5_ILP32_OFFBIG_LDFLAGS, "XBS5_ILP32_OFFBIG_LDFLAGS"},
	{_CS_XBS5_ILP32_OFFBIG_LIBS, "XBS5_ILP32_OFFBIG_LIBS"},
	{_CS_XBS5_ILP32_OFFBIG_LINTFLAGS, "XBS5_ILP32_OFFBIG_LINTFLAGS"},
	{_CS_XBS5_LP64_OFF64_CFLAGS, "XBS5_LP64_OFF64_CFLAGS"},
	{_CS_XBS5_LP64_OFF64_LDFLAGS, "XBS5_LP64_OFF64_LDFLAGS"},
	{_CS_XBS5_LP64_OFF64_LIBS, "XBS5_LP64_OFF64_LIBS"},
	{_CS_XBS5_LP64_OFF64_LINTFLAGS, "XBS5_LP64_OFF64_LINTFLAGS"},
	{_CS_XBS5_LPBIG_OFFBIG_CFLAGS, "XBS5_LPBIG_OFFBIG_CFLAGS"},
	{_CS_XBS5_LPBIG_OFFBIG_LDFLAGS, "XBS5_LPBIG_OFFBIG_LDFLAGS"},
	{_CS_XBS5_LPBIG_OFFBIG_LIBS, "XBS5_LPBIG_OFFBIG_LIBS"},
	{_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS, "XBS5_LPBIG_OFFBIG_LINTFLAGS"},
	{_CS_GNU_LIBC_VERSION, "GNU_LIBC_VERSION"},
	{_CS_GNU_LIBPTHREAD_VERSION, "GNU_LIBPTHREAD_VERSION"},
};

char *TCID = "confstr01";
int TST_TOTAL = ARRAY_SIZE(test_cases);

static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	int lc;
	int i;
	char *buf;
	int len;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(confstr(test_cases[i].value, NULL, (size_t)0));

			if (TEST_RETURN != 0) {
				len = TEST_RETURN;
				buf = SAFE_MALLOC(cleanup, len);
				TEST(confstr(test_cases[i].value, buf, len));

				if (TEST_RETURN != len || buf[len-1] != '\0') {
					tst_brkm(TBROK, cleanup,
						 "confstr :%s failed",
						 test_cases[i].name);
				} else {
					tst_resm(TPASS, "confstr %s = '%s'",
						 test_cases[i].name, buf);
				}
				free(buf);
			} else {
				if (TEST_ERRNO == EINVAL) {
					tst_resm(TCONF,
						 "confstr %s not supported",
						 test_cases[i].name);
				} else {
					tst_resm(TFAIL,
						 "confstr %s failed",
						 test_cases[i].name);
				}
			}
		}
	}

	cleanup();

	tst_exit();
}

static void setup(void)
{
	TEST_PAUSE;
}

static void cleanup(void)
{
}
