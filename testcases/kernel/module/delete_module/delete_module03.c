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
 *      Verify that, delete_module(2) returns -1 and sets errno to EWOULDBLOCK,
 *      if tried to remove a module while other modules depend on this module.
 *
 */

#include <errno.h>
#include "test.h"
#include "old_module.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

#define DUMMY_MOD		"dummy_del_mod"
#define DUMMY_MOD_KO		"dummy_del_mod.ko"
#define DUMMY_MOD_DEP		"dummy_del_mod_dep"
#define DUMMY_MOD_DEP_KO	"dummy_del_mod_dep.ko"

static int dummy_mod_loaded;
static int dummy_mod_dep_loaded;

char *TCID = "delete_module03";

int TST_TOTAL = 1;

static void setup();
static void cleanup(void);

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(ltp_syscall(__NR_delete_module, DUMMY_MOD, 0));

		if (TEST_RETURN < 0) {
			switch (errno) {
			case EWOULDBLOCK:
				tst_resm(TPASS | TTERRNO,
					 "delete_module() failed as expected");
			break;
			default:
				tst_resm(TFAIL | TTERRNO, "delete_module() "
					 "failed unexpectedly; expected: "
					 "%d - %s", EWOULDBLOCK,
					 strerror(EWOULDBLOCK));
			break;
			}
		} else {
			tst_resm(TFAIL, "delete_module()"
				 "succeeded unexpectedly");
			dummy_mod_loaded = 0;
			/*
			 * insmod DUMMY_MOD_KO again in case running
			 * with -i option
			 */
			tst_module_load(cleanup, DUMMY_MOD_KO, NULL);
			dummy_mod_loaded = 1;
		}
	}

	cleanup();
	tst_exit();

}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	/* Load first kernel module */
	tst_module_load(cleanup, DUMMY_MOD_KO, NULL);
	dummy_mod_loaded = 1;

	/* Load dependant kernel module */
	tst_module_load(cleanup, DUMMY_MOD_DEP_KO, NULL);
	dummy_mod_dep_loaded = 1;

	TEST_PAUSE;
}

static void cleanup(void)
{
	/* Unload dependent kernel module */
	if (dummy_mod_dep_loaded == 1)
		tst_module_unload(NULL, DUMMY_MOD_DEP_KO);

	/* Unload first kernel module */
	if (dummy_mod_loaded == 1)
		tst_module_unload(NULL, DUMMY_MOD_KO);
}
