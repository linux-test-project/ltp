/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
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
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * DESCRIPTION
 *	check mkdir() with various error conditions that should produce
 *	EFAULT, ENAMETOOLONG, EEXIST, ENOENT and ENOTDIR
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

static void setup(void);
struct test_case_t;
static void mkdir_verify(struct test_case_t *tc);
static void bad_addr_setup(struct test_case_t *tc);
static void cleanup(void);

#define TST_EEXIST	"tst_eexist"
#define TST_ENOENT	"tst_enoent/tst"
#define TST_ENOTDIR	"tst_enotdir/tst"
#define MODE		0777

char *TCID = "mkdir03";

static char long_dir[PATH_MAX+2];
static int exp_enos[] = { EFAULT, ENAMETOOLONG, EEXIST, ENOENT, ENOTDIR, 0 };

static struct test_case_t {
	char *pathname;
	int mode;
	int exp_errno;
	void (*setupfunc) (struct test_case_t *tc);
} TC[] = {
#if !defined(UCLINUX)
	{NULL, MODE, EFAULT, bad_addr_setup},
#endif
	{long_dir, MODE, ENAMETOOLONG, NULL},
	{TST_EEXIST, MODE, EEXIST, NULL},
	{TST_ENOENT, MODE, ENOENT, NULL},
	{TST_ENOTDIR, MODE, ENOTDIR, NULL},
};

int TST_TOTAL = ARRAY_SIZE(TC);

int main(int ac, char **av)
{
	int i, lc;
	char *msg;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			mkdir_verify(&TC[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	tst_tmpdir();

	memset(long_dir, 'a', PATH_MAX+1);

	SAFE_TOUCH(cleanup, TST_EEXIST, MODE, NULL);

	SAFE_TOUCH(cleanup, "tst_enotdir", MODE, NULL);
}

#if !defined(UCLINUX)
static void bad_addr_setup(struct test_case_t *tc)
{
	tc->pathname = SAFE_MMAP(cleanup, 0, 1, PROT_NONE,
				 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}
#endif

static void mkdir_verify(struct test_case_t *tc)
{
	if (tc->setupfunc != NULL)
		tc->setupfunc(tc);

	TEST(mkdir(tc->pathname, tc->mode));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "mkdir() returned %ld, expected -1, errno=%d",
			 TEST_RETURN, tc->exp_errno);
		return;
	}

	TEST_ERROR_LOG(TEST_ERRNO);

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO, "mkdir() failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "mkdir() failed unexpectedly; expected: %d - %s",
			 tc->exp_errno, strerror(tc->exp_errno));
	}
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
