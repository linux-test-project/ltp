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
 * Test that root can change the real and effective uid to an
 * unpriviledged user.
 */

#include <errno.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/wait.h>

#include "test.h"
#include "compat_16.h"

TCID_DEFINE(setreuid04);

static uid_t neg_one = -1;

static struct passwd nobody, root;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	uid_t *real_uid;
	uid_t *eff_uid;
	struct passwd *exp_real_usr;
	struct passwd *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&neg_one, &neg_one, &root, &root, "After setreuid(-1, nobody),"}, {
&nobody.pw_uid, &nobody.pw_uid, &nobody, &nobody,
		    "After setreuid(-1, -1),"},};

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
		int i, pid;

		tst_count = 0;

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork failed");
		} else if (pid == 0) {	/* child */

			for (i = 0; i < TST_TOTAL; i++) {

				/* Set the real or effective user id */
				TEST(SETREUID(cleanup, *test_data[i].real_uid,
					      *test_data[i].eff_uid));

				if (TEST_RETURN != -1) {
					tst_resm(TPASS, "setreuid(%d, %d) "
						 "succeeded as expected.",
						 *test_data[i].real_uid,
						 *test_data[i].eff_uid);
				} else {
					tst_resm(TFAIL, "setreuid(%d, %d) "
						 "did not return as expected.",
						 *test_data[i].real_uid,
						 *test_data[i].eff_uid);
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

	root = *(getpwnam("root"));
	UID16_CHECK(root.pw_uid, setreuid, cleanup);

	nobody = *(getpwnam("nobody"));
	UID16_CHECK(nobody.pw_uid, setreuid, cleanup);

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
