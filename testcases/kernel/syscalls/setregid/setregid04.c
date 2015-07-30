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
 * Test setregid() when executed by root.
 */

#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "compat_16.h"

TCID_DEFINE(setregid04);

static gid_t neg_one = -1;

static struct group users_gr, daemon_gr, root_gr, bin_gr;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	gid_t *real_gid;
	gid_t *eff_gid;
	struct group *exp_real_usr;
	struct group *exp_eff_usr;
	const char *test_msg;
} test_data[] = {
	{
	&root_gr.gr_gid, &root_gr.gr_gid, &root_gr, &root_gr,
		    "After setregid(root, root),"}, {
	&users_gr.gr_gid, &neg_one, &users_gr, &root_gr,
		    "After setregid(users, -1)"}, {
	&root_gr.gr_gid, &neg_one, &root_gr, &root_gr,
		    "After setregid(root,-1),"}, {
	&neg_one, &neg_one, &root_gr, &root_gr,
		    "After setregid(-1, -1),"}, {
	&neg_one, &root_gr.gr_gid, &root_gr, &root_gr,
		    "After setregid(-1, root)"}, {
	&root_gr.gr_gid, &neg_one, &root_gr, &root_gr,
		    "After setregid(root, -1),"}, {
	&daemon_gr.gr_gid, &users_gr.gr_gid, &daemon_gr, &users_gr,
		    "After setregid(daemon, users)"}, {
	&neg_one, &neg_one, &daemon_gr, &users_gr,
		    "After setregid(-1, -1)"}, {
	&neg_one, &users_gr.gr_gid, &daemon_gr, &users_gr,
		    "After setregid(-1, users)"}
};

int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);

static void setup(void);
static void cleanup(void);
static void gid_verify(struct group *ru, struct group *eu, const char *when);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int i;

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			/* Set the real or effective group id */
			TEST(SETREGID(cleanup, *test_data[i].real_gid,
				      *test_data[i].eff_gid));

			if (TEST_RETURN == -1) {
				tst_resm(TBROK, "setregid(%d, %d) failed",
					 *test_data[i].real_gid,
					 *test_data[i].eff_gid);
			} else {
				gid_verify(test_data[i].exp_real_usr,
					   test_data[i].exp_eff_usr,
					   test_data[i].test_msg);
			}
		}
	}

	cleanup();
	tst_exit();
}

#define SAFE_GETGROUP(GROUPNAME)	\
	if (getgrnam(#GROUPNAME) == NULL) { \
		tst_brkm(TBROK, NULL, "Couldn't find the `" #GROUPNAME "' group"); \
	} \
	GROUPNAME ## _gr = *(getgrnam(#GROUPNAME));

static void setup(void)
{
	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	SAFE_GETGROUP(root);
	SAFE_GETGROUP(users);
	SAFE_GETGROUP(daemon);
	SAFE_GETGROUP(bin);

	TEST_PAUSE;
}

static void cleanup(void)
{
}

static void gid_verify(struct group *rg, struct group *eg, const char *when)
{
	if ((getgid() != rg->gr_gid) || (getegid() != eg->gr_gid)) {
		tst_resm(TFAIL, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_resm(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg->gr_gid, eg->gr_gid);
	} else {
		tst_resm(TPASS, "real or effective gid was modified as "
			 "expected");
	}
}
