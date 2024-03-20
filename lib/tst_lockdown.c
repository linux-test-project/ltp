// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2020-2023
 */

#define TST_NO_DEFAULT_MAIN

#define PATH_LOCKDOWN	"/sys/kernel/security/lockdown"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>

#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_stdio.h"
#include "tst_lockdown.h"
#include "tst_private.h"

#if defined(__powerpc64__) || defined(__ppc64__)
# define SECUREBOOT_VAR "/proc/device-tree/ibm,secure-boot"
# define VAR_DATA_SIZE 4
#else
# define SECUREBOOT_VAR "/sys/firmware/efi/efivars/SecureBoot-8be4df61-93ca-11d2-aa0d-00e098032b8c"
# define VAR_DATA_SIZE 5
#endif

int tst_secureboot_enabled(void)
{
	int fd;
	char data[5];

	if (access(SECUREBOOT_VAR, F_OK)) {
		tst_res(TINFO, "SecureBoot sysfs file not available");
		return -1;
	}

	fd = open(SECUREBOOT_VAR, O_RDONLY);

	if (fd == -1) {
		tst_res(TINFO | TERRNO,
			"Cannot open SecureBoot file");
		return -1;
	} else if (fd < 0) {
		tst_brk(TBROK | TERRNO, "Invalid open() return value %d", fd);
		return -1;
	}
	SAFE_READ(1, fd, data, VAR_DATA_SIZE);
	SAFE_CLOSE(fd);
	tst_res(TINFO, "SecureBoot: %s", data[VAR_DATA_SIZE - 1] ? "on" : "off");
	return data[VAR_DATA_SIZE - 1];
}

int tst_lockdown_enabled(void)
{
	char line[BUFSIZ];
	FILE *file;
	int ret;

	if (access(PATH_LOCKDOWN, F_OK) != 0) {
		char flag;

		/* SecureBoot enabled could mean integrity lockdown (non-mainline version) */
#if defined(__powerpc64__) || defined(__ppc64__)
		flag = tst_kconfig_get("CONFIG_SECURITY_LOCKDOWN_LSM") == 'y';
		flag |= tst_kconfig_get("CONFIG_SECURITY_LOCKDOWN_LSM_EARLY") == 'y';
#else
		flag = tst_kconfig_get("CONFIG_EFI_SECURE_BOOT_LOCK_DOWN") == 'y';
		flag |= tst_kconfig_get("CONFIG_LOCK_DOWN_IN_EFI_SECURE_BOOT") == 'y';
#endif

		if (flag && tst_secureboot_enabled() > 0)
			return 1;

		tst_res(TINFO, "Unable to determine system lockdown state");
		return 0;
	}

	file = SAFE_FOPEN(PATH_LOCKDOWN, "r");
	if (!fgets(line, sizeof(line), file))
		tst_brk(TBROK | TERRNO, "fgets %s", PATH_LOCKDOWN);
	SAFE_FCLOSE(file);

	ret = strstr(line, "[none]") == NULL;
	tst_res(TINFO, "Kernel lockdown: %s", ret ? "on" : "off");

	return ret;
}
