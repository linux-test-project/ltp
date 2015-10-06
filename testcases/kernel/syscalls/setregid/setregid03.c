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
 * Test setregid() when executed by a non-root user.
 */

#include <errno.h>
#include <grp.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <sys/wait.h>

#include "test.h"
#include "compat_16.h"

TCID_DEFINE(setregid03);
static int fail = -1;
static int pass;
static gid_t neg_one = -1;

/* flag to tell parent if child passed or failed. */
static int flag;

struct group users, sys, root, bin;
struct passwd nobody;
/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	gid_t *real_gid;
	gid_t *eff_gid;
	int *exp_ret;
	struct group *exp_real_usr;
	struct group *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&sys.gr_gid, &bin.gr_gid, &pass, &sys, &bin,
		    "After setregid(sys, bin),"}, {
	&neg_one, &sys.gr_gid, &pass, &sys, &sys, "After setregid(-1, sys)"},
	{
	&neg_one, &bin.gr_gid, &pass, &sys, &bin, "After setregid(-1, bin),"},
	{
	&bin.gr_gid, &neg_one, &pass, &bin, &bin, "After setregid(bin, -1),"},
	{
	&neg_one, &neg_one, &pass, &bin, &bin, "After setregid(-1, -1),"}, {
	&neg_one, &bin.gr_gid, &pass, &bin, &bin, "After setregid(-1, bin),"},
	{
	&bin.gr_gid, &neg_one, &pass, &bin, &bin, "After setregid(bin, -1),"},
	{
	&bin.gr_gid, &bin.gr_gid, &pass, &bin, &bin,
		    "After setregid(bin, bin),"}, {
	&sys.gr_gid, &neg_one, &fail, &bin, &bin, "After setregid(sys, -1)"},
	{
	&neg_one, &sys.gr_gid, &fail, &bin, &bin, "After setregid(-1, sys)"},
	{
	&sys.gr_gid, &sys.gr_gid, &fail, &bin, &bin,
		    "After setregid(sys, sys)"},};

int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);

static void setup(void);
static void gid_verify(struct group *ru, struct group *eu, char *when);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		pid_t pid;
		int status, i;

		pass = 0;
		flag = 0;

		tst_count = 0;

		/* set the appropriate ownership values */
		if (SETREGID(NULL, sys.gr_gid, bin.gr_gid) == -1)
			tst_brkm(TBROK, NULL, "Initial setregid failed");

		if (seteuid(nobody.pw_uid) == -1)
			tst_brkm(TBROK, NULL, "Initial seteuid failed");

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, NULL, "fork failed");
		} else if (pid == 0) {	/* child */
			for (i = 0; i < TST_TOTAL; i++) {
				gid_t test_ret;
				/* Set the real or effective group id */
				TEST(SETREGID(NULL, *test_data[i].real_gid,
					      *test_data[i].eff_gid));
				test_ret = TEST_RETURN;

				if (test_ret == *test_data[i].exp_ret) {
					if (test_ret == neg_one) {
						if (TEST_ERRNO != EPERM) {
							tst_resm(TFAIL,
								 "setregid(%d, %d) "
								 "did not set errno "
								 "value as expected.",
								 *test_data
								 [i].real_gid,
								 *test_data
								 [i].eff_gid);
							fail = -1;
							continue;
						} else {
							tst_resm(TPASS,
								 "setregid(%d, %d) "
								 "failed as expected.",
								 *test_data
								 [i].real_gid,
								 *test_data
								 [i].eff_gid);
						}
					} else {
						tst_resm(TPASS,
							 "setregid(%d, %d) "
							 "succeeded as expected.",
							 *test_data[i].real_gid,
							 *test_data[i].eff_gid);
					}
				} else {
					tst_resm(TFAIL, "setregid(%d, %d) "
						 "did not return as expected.",
						 *test_data[i].real_gid,
						 *test_data[i].eff_gid);
					flag = -1;
				}
				if (test_ret == -1) {
				}

				gid_verify(test_data[i].exp_real_usr,
					   test_data[i].exp_eff_usr,
					   test_data[i].test_msg);
			}
			exit(flag);
		} else {	/* parent */
			waitpid(pid, &status, 0);
			if (WEXITSTATUS(status) != 0) {
				tst_resm(TFAIL, "test failed within "
					 "child process.");
			}
		}
	}

	tst_exit();
}

static void setup(void)
{
	struct group *junk;

	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, NULL);

	if (getpwnam("nobody") == NULL)
		tst_brkm(TBROK, NULL, "nobody must be a valid user.");
	nobody = *(getpwnam("nobody"));

#define GET_GID(group) do { \
	junk = getgrnam(#group); \
	if (junk == NULL) { \
		tst_brkm(TBROK, NULL, "%s must be a valid group", #group); \
	} \
	GID16_CHECK(junk->gr_gid, setregid, NULL); \
	group = *(junk); \
} while (0)

	GET_GID(users);
	GET_GID(sys);
	GET_GID(bin);

	TEST_PAUSE;
}

static void gid_verify(struct group *rg, struct group *eg, char *when)
{
	if ((getgid() != rg->gr_gid) || (getegid() != eg->gr_gid)) {
		tst_resm(TFAIL, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_resm(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg->gr_gid, eg->gr_gid);
		flag = -1;
	} else {
		tst_resm(TPASS, "real or effective gid was modified as expected");
	}
}
