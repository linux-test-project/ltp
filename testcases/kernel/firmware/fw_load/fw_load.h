// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef FW_LOAD_H
#define FW_LOAD_H

#include "tst_test.h"

#define MNAME_KO "ltp_fw_load.ko"
#define FW_NAME	"load_tst.fw"
#define FW_SIZE	0x1000
#define FW_NUM 5
#define FW_PATH "/sys/module/firmware_class/parameters/path"
#define DEV_FWNUM "/sys/devices/ltp_fw_load/fwnum"
#define DEV_RESULT "/sys/devices/ltp_fw_load/result"
#define LIB_PATH "/lib/firmware"

struct fw_data {
	char dir[PATH_MAX];
	char file[PATH_MAX];
	int fake;
	int created_dir;
};

static inline void create_firmware(struct fw_data *firmware,
				   int *fw_count, const char *dir)
{
	struct fw_data *fw = &firmware[*fw_count];
	char buf[FW_SIZE];
	int fd;

	snprintf(fw->dir, sizeof(fw->dir), "%s", dir);

	if (access(dir, X_OK) == -1) {
		SAFE_MKDIR(dir, 0755);
		fw->created_dir = 1;
	}

	snprintf(fw->file, sizeof(fw->file), "%s/n%d_%s", dir, *fw_count, FW_NAME);
	memset(buf, *fw_count, FW_SIZE);

	fd = SAFE_OPEN(fw->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, FW_SIZE);
	SAFE_CLOSE(fd);

	(*fw_count)++;
}

static inline void create_fake_firmware(struct fw_data *firmware, int *fw_count)
{
	snprintf(firmware[*fw_count].file, sizeof(firmware[*fw_count].file),
		 "/n%d_%s", *fw_count, FW_NAME);

	firmware[*fw_count].fake = 1;
	(*fw_count)++;
}

static inline void do_test(struct fw_data *firmware, int fw_count)
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

#endif
