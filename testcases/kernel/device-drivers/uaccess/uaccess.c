/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 * Test checks kernel API to safely access user-space memory using
 * copy_to_user(), copy_from_user(), get_user(), put_user() functions.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test.h"
#include "tst_kconfig.h"
#include "old_module.h"
#include "safe_macros.h"

#include "ltp_uaccess.h"

char *TCID = DEV_NAME;

static const char dev_result[]	= "/sys/devices/" DEV_NAME "/result";
static const char dev_tcase[]	= "/sys/devices/" DEV_NAME "/tcase";
static const char module_name[]	= DEV_NAME ".ko";
static int module_loaded;

static void cleanup(void)
{
	if (module_loaded)
		tst_module_unload(NULL, module_name);
}

static int set_ptr_to_sysfs(int id, const void *ptr, const char *descr)
{
	int res;
	tst_resm(TINFO, "TC %d: %s, ptr '%p'", id, descr, ptr);
	SAFE_FILE_PRINTF(cleanup, dev_tcase, "%d %lu", id, (unsigned long) ptr);
	SAFE_FILE_SCANF(cleanup, dev_result, "%d", &res);
	if (res)
		return TFAIL;

	return TPASS;
}

/*
 * Read user-space memory using copy_from_user(), get_user().
 */
static void tc_read_userspace(void)
{
	int res = set_ptr_to_sysfs(TC_READ_USER, test_str,
		"read user-space memory from kernel");

	tst_resm(res, "copy_from_user(), get_user(): strings%sequal",
		(res) ? " not " : " ");
}

/*
 * Write from kernel-space to user-space
 * using copy_to_user(), put_user().
 */
static void tc_write_userspace(void)
{
	char buf[str_size];
	memset(buf, 0, str_size);

	int res = set_ptr_to_sysfs(TC_WRITE_USER, buf,
		"write from kernel-space to user-space");
	if (res) {
		tst_resm(TFAIL, "failed to write from kernel");
		return;
	}

	res = strncmp(buf, test_str, str_size) ? TFAIL : TPASS;
	tst_resm(res, "copy_to_user(), put_user(): strings%sequal",
		(res) ? " not " : " ");
}

int main(int argc, char *argv[])
{
	struct tst_kcmdline_var params = TST_KCMDLINE_INIT("module.sig_enforce");
	struct tst_kconfig_var kconfig = TST_KCONFIG_INIT("CONFIG_MODULE_SIG_FORCE");

	tst_parse_opts(argc, argv, NULL, NULL);

	tst_require_root();

	tst_kcmdline_parse(&params, 1);
	tst_kconfig_read(&kconfig, 1);
	if (params.found || kconfig.choice == 'y')
		tst_brkm(TCONF, tst_exit, "module signature is enforced, skip test");

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_module_load(NULL, module_name, NULL);
	module_loaded = 1;

	tc_read_userspace();
	tc_write_userspace();

	cleanup();
	tst_exit();
}
