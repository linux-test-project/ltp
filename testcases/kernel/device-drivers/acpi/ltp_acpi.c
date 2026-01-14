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
 */

#include <stdio.h>
#include <stdlib.h>

#include "test.h"
#include "tso_module.h"
#include "tso_safe_macros.h"

#include "ltp_acpi.h"

char *TCID = "ltp_acpi";
int TST_TOTAL = ACPI_TC_NUM;

static const char dev_result[]	= "/sys/devices/" ACPI_TEST_NAME "/result";
static const char dev_path[]	= "/sys/devices/" ACPI_TEST_NAME "/path";
static const char dev_str[]	= "/sys/devices/" ACPI_TEST_NAME "/str";
static const char dev_tcase[]	= "/sys/devices/" ACPI_TEST_NAME "/tcase";
static const char dev_acpi_disabled[] = "/sys/devices/" ACPI_TEST_NAME "/acpi_disabled";
static const char module_name[]	= "ltp_acpi_cmds.ko";
static int module_loaded;

static void cleanup(void)
{
	if (module_loaded)
		tst_module_unload(NULL, module_name);
}

static int read_sysfs_file(const char *name, char *buf, int size)
{
	FILE *f = SAFE_FOPEN(cleanup, name, "r");
	char *res = fgets(buf, size, f);
	SAFE_FCLOSE(cleanup, f);
	return (res) ? 0 : 1;
}

static int tc_acpi_str(void)
{
	int res, ret = 0;
	char descr[4096], sysfs_path[4096];

	while (1) {

		SAFE_FILE_PRINTF(cleanup, dev_tcase, "%d", ACPI_TRAVERSE);
		SAFE_FILE_SCANF(cleanup, dev_result, "%d", &res);
		if (res)
			return TFAIL;
		/*
		 * if device has _STR object, we should get
		 * a valid string from 'str' sysfs file and then can
		 * find it in sysfs.
		 */
		if (read_sysfs_file(dev_str, descr, 4096)) {
			/* None of left devices has _STR */
			break;
		}
		tst_resm(TINFO, "read description %s", descr);

		/* device's sysfs path */
		strcpy(sysfs_path, "/sys");
		if (read_sysfs_file(dev_path, sysfs_path + 4, 4092)) {
			/*
			 * Device doesn't have sysfs entry
			 * continue, because others might have it
			 */
			continue;
		}

		strcat(sysfs_path, "/description");
		if (access(sysfs_path, R_OK)) {
			tst_resm(TINFO, "can't find description file '%s'",
				sysfs_path);
			return TFAIL;
		}
		tst_resm(TINFO, "found description file '%s'", sysfs_path);

		char sysfs_descr[4096];
		if (read_sysfs_file(sysfs_path, sysfs_descr, 4096))
			return TFAIL;

		/*
		 * Compare sysfs file and description from test driver
		 */
		int res = strncmp(descr, sysfs_descr, strlen(descr));

		ret |= res ? TFAIL : TPASS;
	}

	return ret;
}

static void test_run(void)
{
	int i, res;

	for (i = 0; i < TST_TOTAL; ++i) {

		if (i == ACPI_TRAVERSE) {
			res = tc_acpi_str();
		} else {
			SAFE_FILE_PRINTF(cleanup, dev_tcase, "%d", i);
			SAFE_FILE_SCANF(cleanup, dev_result, "%d", &res);
			res = res ? TFAIL : TPASS;
		}

		tst_resm(res, "Test-case '%d'", i);
	}
}

int main(int argc, char *argv[])
{
	int acpi_disabled;

	tst_parse_opts(argc, argv, NULL, NULL);
	tst_require_root();
	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_requires_module_signature_disabled();
	tst_module_load(NULL, module_name, NULL);
	module_loaded = 1;

	SAFE_FILE_SCANF(cleanup, dev_acpi_disabled, "%d", &acpi_disabled);
	if (acpi_disabled)
		tst_brkm(TCONF, cleanup, "ACPI is disabled on the system");

	test_run();

	cleanup();

	tst_exit();
}
