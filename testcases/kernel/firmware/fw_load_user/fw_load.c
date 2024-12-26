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
 * Author:
 * Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 * Test checks device firmware loading.
 */

#define _GNU_SOURCE
#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "test.h"
#include "tst_kconfig.h"
#include "safe_macros.h"
#include "old_module.h"

/* number of test firmware files */
#define FW_FILES	5

char *TCID = "fw_load";
int TST_TOTAL = FW_FILES;

static int fw_size = 0x1000;

static const char fw_name[]	= "load_tst.fw";
static const char module_name[]	= "ltp_fw_load.ko";

/* paths to module's sysfs files */
static const char dev_fwnum[]	= "/sys/devices/ltp_fw_load/fwnum";
static const char dev_result[]	= "/sys/devices/ltp_fw_load/result";

struct fw_file_info {
	char *file;
	char *dir;
	int fake;
	int remove_dir;
	int remove_file;
};

static struct fw_file_info fw[FW_FILES];
static int fw_num;

/* test options */
static char *narg;
static int nflag;
static int skip_cleanup;
static int verbose;
static const option_t options[] = {
	{"n:", &nflag, &narg},
	{"s", &skip_cleanup, NULL},
	{"v", &verbose, NULL},
	{NULL, NULL, NULL}
};

static void help(void);
static void setup(int argc, char *argv[]);
static void test_run(void);
static void cleanup(void);

/*
 * create firmware files in the fw_paths
 * @fw_paths: it must be termintated by a NULL pointer
 */
static void create_firmware(char *const fw_paths[]);

int main(int argc, char *argv[])
{
	setup(argc, argv);

	test_run();

	cleanup();

	tst_exit();
}

static void help(void)
{
	printf("  -n x    Write x bytes to firmware file, default is %d\n",
		fw_size);
	printf("  -s      Skip cleanup\n");
	printf("  -v      Verbose\n");
}

void setup(int argc, char *argv[])
{
	struct tst_kcmdline_var params = TST_KCMDLINE_INIT("module.sig_enforce");
	struct tst_kconfig_var kconfig = TST_KCONFIG_INIT("CONFIG_MODULE_SIG_FORCE");

	tst_parse_opts(argc, argv, options, help);

	if (nflag) {
		if (sscanf(narg, "%i", &fw_size) != 1)
			tst_brkm(TBROK, NULL, "-n option arg is not a number");
		if (fw_size < 0)
			tst_brkm(TBROK, NULL, "-n option arg is less than 0");
	}

	tst_require_root();

	tst_kcmdline_parse(&params, 1);
	tst_kconfig_read(&kconfig, 1);
	if (params.found || kconfig.choice == 'y')
		tst_brkm(TCONF, tst_exit, "module signature is enforced, skip test");

	char fw_size_param[19];
	snprintf(fw_size_param, 19, "fw_size=%d", fw_size);
	char *const mod_params[2] = { fw_size_param, NULL };
	tst_module_load(NULL, module_name, mod_params);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* get current Linux version and make firmware paths */
	struct utsname uts_name;
	uname(&uts_name);

	/* 4 firmware paths + NULL */
	char *fw_paths[5] = { "/lib/firmware", "/lib/firmware/updates" };
	SAFE_ASPRINTF(cleanup, &fw_paths[2], "%s/%s", fw_paths[0], uts_name.release);
	SAFE_ASPRINTF(cleanup, &fw_paths[3], "%s/%s", fw_paths[1], uts_name.release);

	/* create firmware in the hard coded firmware search paths */
	create_firmware(fw_paths);

	free(fw_paths[2]);
	free(fw_paths[3]);

	/* make non-existent firmware file */
	SAFE_ASPRINTF(cleanup, &fw[fw_num].file, "/n%d_%s", fw_num, fw_name);
	fw[fw_num].fake = 1;
	++fw_num;
}

static void test_run(void)
{
	/* initiate firmware requests */
	SAFE_FILE_PRINTF(cleanup, dev_fwnum, "%d", fw_num);

	/* get module results by reading result bit mask */
	int result = 0;
	SAFE_FILE_SCANF(cleanup, dev_result, "%d", &result);

	int i, fail, offset;
	for (i = 0; i < fw_num; ++i) {
		fail = (result & (1 << i)) == 0 && !fw[i].fake;
		offset = (fw[i].dir) ? strlen(fw[i].dir) : 0;
		tst_resm((fail) ? TFAIL : TPASS,
			"Expect: %s load firmware '...%s'",
			(fw[i].fake) ? "can't" : "can",
			fw[i].file + offset);
	}
}

static void cleanup(void)
{
	if (skip_cleanup)
		return;

	int i;
	/* remove subdirs first and then upper level dirs */
	for (i = fw_num - 1; i >= 0; --i) {
		if (fw[i].remove_file && remove(fw[i].file) == -1)
			tst_resm(TWARN, "Can't remove: %s", fw[i].file);
		free(fw[i].file);

		if (fw[i].remove_dir && remove(fw[i].dir) == -1)
			tst_resm(TWARN, "Can't remove %s", fw[i].dir);
		free(fw[i].dir);
	}

	tst_module_unload(NULL, module_name);
}

static void create_firmware(char *const fw_paths[])
{
	int i = 0;
	while (fw_paths[i] != NULL) {
		struct fw_file_info *fi = &fw[fw_num];
		fi->dir = strdup(fw_paths[i]);
		if (access(fi->dir, X_OK) == -1) {
			/* create dir */
			SAFE_MKDIR(cleanup, fi->dir, 0755);
			fi->remove_dir = 1;
		}

		/* create test firmware file */
		SAFE_ASPRINTF(cleanup, &fi->file, "%s/n%d_%s", fi->dir, fw_num, fw_name);

		FILE *f = SAFE_FOPEN(cleanup, fi->file, "w");
		fi->remove_file = 1;
		int k, byte = fw_num;
		++fw_num;
		for (k = 0; k < fw_size; ++k)
			fputc(byte, f);
		SAFE_FCLOSE(cleanup, f);
		++i;
	}
}
