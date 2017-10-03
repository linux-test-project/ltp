/*
 *
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Ported by John George
 */

/*
 * Test setreuid() when executed by an unpriviledged user.
 */

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>

#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

#define FAILED  1

TCID_DEFINE(setreuid03);

static int fail = -1;
static int pass;
static uid_t neg_one = -1;

static struct passwd nobody, bin, root;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

static struct test_data_t {
	uid_t *real_uid;
	uid_t *eff_uid;
	int *exp_ret;
	struct passwd *exp_real_usr;
	struct passwd *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&nobody.pw_uid, &nobody.pw_uid, &pass, &nobody, &nobody,
		    "After setreuid(nobody, nobody),"}, {
	&neg_one, &nobody.pw_uid, &pass, &nobody, &nobody,
		    "After setreuid(-1, nobody),"}, {
	&nobody.pw_uid, &neg_one, &pass, &nobody, &nobody,
		    "After setreuid(nobody, -1),"}, {
	&neg_one, &neg_one, &pass, &nobody, &nobody, "After setreuid(-1, -1),"},
	{
	&neg_one, &root.pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(-1, root),"}, {
	&root.pw_uid, &neg_one, &fail, &nobody, &nobody,
		    "After setreuid(root, -1),"}, {
	&root.pw_uid, &root.pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(root, root),"}, {
	&root.pw_uid, &nobody.pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(root, nobody),"}, {
	&root.pw_uid, &bin.pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(root, nobody),"}, {
	&bin.pw_uid, &root.pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(bin, root),"}, {
	&bin.pw_uid, &neg_one, &fail, &nobody, &nobody,
		    "After setreuid(bin, -1),"}, {
	&bin.pw_uid, &bin.pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(bin, bin,),"}, {
	&bin.pw_uid, &nobody.pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(bin, nobody),"}, {
	&nobody.pw_uid, &bin.pw_uid, &fail, &nobody, &nobody,
		    "After setreuid(nobody, bin),"},};

int TST_TOTAL = ARRAY_SIZE(test_data);

static void setup(void);
static void cleanup(void);
static void uid_verify(struct passwd *, struct passwd *, char *);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int i;

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			/* Set the real or effective user id */
			TEST(SETREUID(cleanup, *test_data[i].real_uid,
				      *test_data[i].eff_uid));

			if (TEST_RETURN == *test_data[i].exp_ret) {
				if (TEST_RETURN == neg_one) {
					if (TEST_ERRNO != EPERM) {
						tst_resm(TFAIL,
							 "setreuid(%d, %d) "
							 "did not set errno "
							 "value as expected.",
							 *test_data[i].real_uid,
							 *test_data[i].eff_uid);
						continue;
					}
					tst_resm(TPASS, "setreuid(%d, %d) "
						 "failed as expected.",
						 *test_data[i].real_uid,
						 *test_data[i].eff_uid);
				} else {
					tst_resm(TPASS, "setreuid(%d, %d) "
						 "succeeded as expected.",
						 *test_data[i].real_uid,
						 *test_data[i].eff_uid);
				}
			} else {
				tst_resm(TFAIL, "setreuid(%d, %d) "
					 "did not return as expected.",
					 *test_data[i].real_uid,
					 *test_data[i].eff_uid);
			}

			if (TEST_RETURN == -1) {
			}
			uid_verify(test_data[i].exp_real_usr,
				   test_data[i].exp_eff_usr,
				   test_data[i].test_msg);
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	if (getpwnam("nobody") == NULL)
		tst_brkm(TBROK, NULL, "nobody must be a valid user.");

	if (getpwnam("bin") == NULL)
		tst_brkm(TBROK, NULL, "bin must be a valid user.");

	root = *(getpwnam("root"));
	UID16_CHECK(root.pw_uid, setreuid, cleanup);

	nobody = *(getpwnam("nobody"));
	UID16_CHECK(nobody.pw_uid, setreuid, cleanup);

	bin = *(getpwnam("bin"));
	UID16_CHECK(bin.pw_uid, setreuid, cleanup);

	SAFE_SETUID(NULL, nobody.pw_uid);

	TEST_PAUSE;
}

static void cleanup(void)
{
}

static void uid_verify(struct passwd *ru, struct passwd *eu, char *when)
{
	if ((getuid() != ru->pw_uid) || (geteuid() != eu->pw_uid)) {
		tst_resm(TFAIL, "ERROR: %s real uid = %d; effective uid = %d",
			 when, getuid(), geteuid());
		tst_resm(TINFO, "Expected: real uid = %d; effective uid = %d",
			 ru->pw_uid, eu->pw_uid);
	}
}
