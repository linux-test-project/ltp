/*
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
 * Test the setreuid() feature, verifying the role of the saved-set-uid
 * and setreuid's effect on it.
 */

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "test.h"
#include "compat_16.h"

TCID_DEFINE(setreuid05);

static int fail = -1;
static int pass;
static uid_t neg_one = -1;

static struct passwd nobody, daemonpw, root, bin;

struct test_data_t {
	uid_t *real_uid;
	uid_t *eff_uid;
	int *exp_ret;
	struct passwd *exp_real_usr;
	struct passwd *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&nobody.pw_uid, &root.pw_uid, &pass, &nobody, &root, "Initially"}, {
	&neg_one, &nobody.pw_uid, &pass, &nobody, &nobody,
		    "After setreuid(-1, nobody),"}, {
	&neg_one, &root.pw_uid, &pass, &nobody, &root,
		    "After setreuid(-1, root),"}, {
	&daemonpw.pw_uid, &neg_one, &pass, &daemonpw, &root,
		    "After setreuid(daemon, -1),"}, {
	&neg_one, &bin.pw_uid, &pass, &daemonpw, &bin,
		    "After setreuid(-1, bin),"}, {
	&neg_one, &root.pw_uid, &fail, &daemonpw, &bin,
		    "After setreuid(-1, root),"}, {
	&neg_one, &nobody.pw_uid, &fail, &daemonpw, &bin,
		    "After setreuid(-1, nobody),"}, {
	&neg_one, &daemonpw.pw_uid, &pass, &daemonpw, &daemonpw,
		    "After setreuid(-1, daemon),"}, {
	&neg_one, &bin.pw_uid, &pass, &daemonpw, &bin,
		    "After setreuid(-1, bin),"}, {
	&bin.pw_uid, &daemonpw.pw_uid, &pass, &bin, &daemonpw,
		    "After setreuid(bin, daemon),"}, {
	&neg_one, &bin.pw_uid, &pass, &bin, &bin, "After setreuid(-1, bin),"},
	{
	&neg_one, &daemonpw.pw_uid, &pass, &bin, &daemonpw,
		    "After setreuid(-1, daemon),"}, {
	&daemonpw.pw_uid, &neg_one, &pass, &daemonpw, &daemonpw,
		    "After setreuid(daemon, -1),"}, {
	&neg_one, &bin.pw_uid, &fail, &daemonpw, &daemonpw,
		    "After setreuid(-1, bin),"},};

int TST_TOTAL = ARRAY_SIZE(test_data);

static void setup(void);
static void cleanup(void);
static void uid_verify(struct passwd *, struct passwd *, char *);

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	pass = 0;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int i, pid;

		tst_count = 0;

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork failed");
		} else if (pid == 0) {	/* child */
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
								 *test_data
								 [i].real_uid,
								 *test_data
								 [i].eff_uid);
							continue;
						}
						tst_resm(TPASS,
							 "setreuid(%d, %d) "
							 "failed as expected.",
							 *test_data[i].real_uid,
							 *test_data[i].eff_uid);
					} else {
						tst_resm(TPASS,
							 "setreuid(%d, %d) "
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
			tst_exit();
		} else {	/* parent */
			tst_record_childstatus(cleanup, pid);
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

	if (getpwnam("daemon") == NULL)
		tst_brkm(TBROK, NULL, "daemon must be a valid user.");

	if (getpwnam("bin") == NULL)
		tst_brkm(TBROK, NULL, "bin must be a valid user.");

	nobody = *(getpwnam("nobody"));
	UID16_CHECK(nobody.pw_uid, setreuid, cleanup);

	daemonpw = *(getpwnam("daemon"));
	UID16_CHECK(daemonpw.pw_uid, setreuid, cleanup);

	root = *(getpwnam("root"));
	UID16_CHECK(root.pw_uid, setreuid, cleanup);

	bin = *(getpwnam("bin"));
	UID16_CHECK(bin.pw_uid, setreuid, cleanup);

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
