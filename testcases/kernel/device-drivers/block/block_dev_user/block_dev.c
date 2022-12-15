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
 * Test checks block device kernel API.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "test.h"
#include "safe_macros.h"
#include "old_module.h"

char *TCID = "block_dev";
int TST_TOTAL = 9;

static const char module_name[]	= "ltp_block_dev.ko";
static const char dev_result[]	= "/sys/devices/ltp_block_dev/result";
static const char dev_tcase[]	= "/sys/devices/ltp_block_dev/tcase";
static int module_loaded;

static int run_all_testcases;
static const option_t options[] = {
	{"a", &run_all_testcases, NULL},
	{NULL, NULL, NULL}
};

static void cleanup(void)
{
	if (module_loaded)
		tst_module_unload(NULL, module_name);
}

static void help(void)
{
	printf("  -a      Run all test-cases (can crash the kernel)\n");
}

void setup(int argc, char *argv[])
{
	tst_parse_opts(argc, argv, options, help);

	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);
}

static void test_run(void)
{
	int off = 0;
	/*
	 * test-cases #8 and #9 can crash the kernel.
	 * We have to wait for kernel fix where register_blkdev() &
	 * unregister_blkdev() checks the input device name parameter
	 * against NULL pointer.
	 */
	if (!run_all_testcases)
		off = 2;

	tst_module_load(cleanup, module_name, NULL);
	module_loaded = 1;

	int i, pass = 0;
	for (i = 0; i < TST_TOTAL - off; ++i) {
		SAFE_FILE_PRINTF(cleanup, dev_tcase, "%d", i + 1);
		SAFE_FILE_SCANF(cleanup, dev_result, "%d", &pass);
		tst_resm((pass) ? TPASS : TFAIL, "Test-case '%d'", i + 1);
	}
}

int main(int argc, char *argv[])
{
	setup(argc, argv);

	test_run();

	cleanup();

	tst_exit();
}
