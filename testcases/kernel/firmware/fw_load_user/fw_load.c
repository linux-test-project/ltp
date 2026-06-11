// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *	Alexey Kodanev <alexey.kodanev@oracle.com>
 * Copyright (c) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verifies that the kernel firmware loader (``CONFIG_FW_LOADER``)
 * can find and load firmware files from the standard search paths.
 *
 * A helper kernel module (``ltp_fw_load.ko``) registers a virtual
 * device that calls :kernel_doc:`request_firmware` for a set of
 * numbered firmware blobs. Each blob is verified in-kernel against
 * its expected size and byte pattern.
 *
 * [Algorithm]
 *
 * - Load the helper module with ``fw_size`` matching the blob size
 * - Create firmware files in the standard firmware search
 *   directories:
 *
 *   - ``/lib/firmware/``
 *   - ``/lib/firmware/<kernel-release>/``
 *   - ``/lib/firmware/updates/``
 *   - ``/lib/firmware/updates/<kernel-release>/``
 *
 * - Add one fake firmware entry that has no file on disk
 * - Write the firmware count to ``/sys/devices/ltp_fw_load/fwnum``
 *   to trigger :kernel_doc:`request_firmware` calls in-kernel
 * - Read the result bitmask from ``/sys/devices/ltp_fw_load/result``
 * - Verify that every real firmware file was loaded successfully
 *   and that the fake entry was correctly rejected
 */

#include <sys/utsname.h>
#include "tst_test.h"
#include "tst_module.h"
#include "fw_load.h"

static int module_loaded;
static int fw_count;

static struct fw_data {
	char dir[PATH_MAX];
	char file[PATH_MAX];
	int fake;
	int created_dir;
} firmware[FW_NUM];

static void create_firmware(const char *dir)
{
	struct fw_data *fw = &firmware[fw_count];
	char buf[FW_SIZE];
	int fd = -1;

	snprintf(fw->dir, sizeof(fw->dir), "%s", dir);
	if (access(dir, X_OK) == -1) {
		SAFE_MKDIR(dir, 0755);
		fw->created_dir = 1;
	}

	snprintf(fw->file, sizeof(fw->file), "%s/n%d_%s", dir, fw_count, FW_NAME);
	memset(buf, fw_count, FW_SIZE);

	fd = SAFE_OPEN(fw->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, FW_SIZE);
	SAFE_CLOSE(fd);

	fw_count++;
}

static void run(void)
{
	struct fw_data *fw;
	int result = 0;
	int pass, offset;

	SAFE_FILE_PRINTF(DEV_FWNUM, "%d", fw_count);
	SAFE_FILE_SCANF(DEV_RESULT, "%d", &result);

	for (int i = 0; i < fw_count; i++) {
		fw = &firmware[i];

		pass = result & (1 << i);
		offset = fw->dir[0] ? strlen(fw->dir) : 0;

		if (fw->fake) {
			tst_res(pass ? TFAIL : TPASS,
				"Firmware '%s' correctly not loaded",
				fw->file + offset);
		} else {
			tst_res(pass ? TPASS : TFAIL,
				"Firmware '%s' loaded",
				fw->file + offset);
		}
	}
}

static void setup(void)
{
	char fw_dir[PATH_MAX];
	char fw_size_param[32];
	struct utsname name;

	if (access(LIB_PATH, W_OK) == -1)
		tst_brk(TCONF, "Skipping test due to read-only %s", LIB_PATH);

	tst_requires_module_signature_disabled();

	snprintf(fw_size_param, sizeof(fw_size_param), "fw_size=%d", FW_SIZE);
	char *const mod_params[] = {fw_size_param, NULL};

	tst_module_load(MNAME_KO, mod_params);
	module_loaded = 1;

	create_firmware(LIB_PATH);

	uname(&name);

	snprintf(fw_dir, sizeof(fw_dir), "%s/%s", LIB_PATH, name.release);
	create_firmware(fw_dir);

	snprintf(fw_dir, sizeof(fw_dir), "%s/updates", LIB_PATH);
	create_firmware(fw_dir);

	snprintf(fw_dir, sizeof(fw_dir), "%s/updates/%s", LIB_PATH, name.release);
	create_firmware(fw_dir);

	/* add a fake file */
	snprintf(firmware[fw_count].file, sizeof(firmware[fw_count].file),
		 "/n%d_%s", fw_count, FW_NAME);

	firmware[fw_count].fake = 1;
	fw_count++;
}

static void cleanup(void)
{
	struct fw_data *fw;

	for (int i = fw_count - 1; i >= 0; i--) {
		fw = &firmware[i];

		if (access(fw->file, F_OK) != -1)
			SAFE_UNLINK(fw->file);

		if (fw->created_dir)
			remove(fw->dir);
	}

	if (module_loaded)
		tst_module_unload(MNAME_KO);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_FW_LOADER=y|CONFIG_FW_LOADER=m",
		NULL,
	},
};
