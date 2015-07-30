/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*
 *    AUTHOR: Madhu T L <madhu.tarikere@wipro.com>
 *
 *    DESCRIPTION
 *      Verify that,
 *      1. delete_module(2) returns -1 and sets errno to ENOENT for nonexistent
 *	   module entry.
 *      2. delete_module(2) returns -1 and sets errno to EFAULT, if
 *         module name parameter is outside program's accessible address space.
 *      3. delete_module(2) returns -1 and sets errno to EPERM, if effective
 *         user id of the caller is not superuser.
 */

#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#if HAVE_LINUX_MODULE_H
#include <linux/module.h>
#else
/* As per http://tomoyo.sourceforge.jp/cgi-bin/lxr/source/include/linux/moduleparam.h?a=ppc#L17 ... */
#define MODULE_NAME_LEN	( 64 - sizeof(unsigned long) )
#endif
#include <sys/mman.h>
#include "test.h"
#include "safe_macros.h"
#include "linux_syscall_numbers.h"

#define NULLMODNAME	""
#define BASEMODNAME	"dummy"
#define LONGMODNAMECHAR	'm'	/* Arbitrarily selected */

char *TCID = "delete_module02";

static char nobody_uid[] = "nobody";
struct passwd *ltpuser;
static char longmodname[MODULE_NAME_LEN];
static char modname[20];

static void setup(void);
static void cleanup(void);
static void setup1(void);
static void cleanup1(void);

static struct test_case_t {
	char *modname;
	int experrno;
	char *desc;
	void (*setup) (void);
	void (*cleanup) (void);
} tdat[] = {
	{ modname, ENOENT, "nonexistent module", NULL, NULL},
	{ NULLMODNAME, ENOENT, "null terminated module name", NULL, NULL},
	{ (char *)-1, EFAULT, "module name outside program's "
	  "accessible address space", NULL, NULL},
	{ longmodname, ENOENT, "long module name", NULL, NULL},
	{ modname, EPERM, "non-superuser", setup1, cleanup1},
};

int TST_TOTAL = ARRAY_SIZE(tdat);

int main(int argc, char **argv)
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {
			if (tdat[i].setup)
				tdat[i].setup();

			tst_resm(TINFO, "test %s", tdat[i].desc);
			TEST(ltp_syscall(__NR_delete_module,
			     tdat[i].modname, 0));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "delete_module() "
					 "succeeded unexpectedly");
			} else if (TEST_ERRNO == tdat[i].experrno) {
				tst_resm(TPASS | TTERRNO,
					 "delete_module() failed as expected");
			} else {
				tst_resm(TFAIL | TTERRNO, "delete_module() "
					 "failed unexpectedly; expected: "
					 "%d - %s", tdat[i].experrno,
					 strerror(tdat[i].experrno));
			}
			if (tdat[i].cleanup)
				tdat[i].cleanup();
		}
	}

	cleanup();
	tst_exit();
}

static void setup1(void)
{
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void cleanup1(void)
{
	SAFE_SETEUID(cleanup, 0);
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	ltpuser = SAFE_GETPWNAM(cleanup, nobody_uid);

	TEST_PAUSE;

	/* Initialize longmodname to LONGMODNAMECHAR character */
	memset(longmodname, LONGMODNAMECHAR, MODULE_NAME_LEN - 1);

	/* Get unique module name for each child process */
	if (sprintf(modname, "%s_%d", BASEMODNAME, getpid()) <= 0)
		tst_brkm(TBROK, NULL, "Failed to initialize module name");

	tdat[2].modname = SAFE_MMAP(cleanup, 0, 1, PROT_NONE,
				    MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void cleanup(void)
{
}
