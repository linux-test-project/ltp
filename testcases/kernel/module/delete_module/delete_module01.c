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
 *    DESCRIPTION:
 *    	Basic tests for delete_module(2)
 *    	1) insmod dummy_del_mod.ko
 *    	2) call delete_module(2) to remove dummy_del_mod.ko
 */

#include <errno.h>
#include "test.h"
#include "old_module.h"
#include "safe_macros.h"
#include "linux_syscall_numbers.h"

#define MODULE_NAME	"dummy_del_mod"
#define MODULE_NAME_KO	"dummy_del_mod.ko"

static void setup(void);
static void cleanup(void);


char *TCID = "delete_module01";
int TST_TOTAL = 1;
static int module_loaded;

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		/* insert dummy_del_mod.ko */
		if (module_loaded == 0) {
			tst_module_load(NULL, MODULE_NAME_KO, NULL);
			module_loaded = 1;
		}

		TEST(ltp_syscall(__NR_delete_module, MODULE_NAME, 0));
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "delete_module() failed to "
				 "remove module entry for %s ", MODULE_NAME);
		} else {
			tst_resm(TPASS, "delete_module() successful");
			module_loaded = 0;
		}

	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (module_loaded == 1)
		tst_module_unload(NULL, MODULE_NAME_KO);
}
