// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2020-2024
 */

#define TST_NO_DEFAULT_MAIN

#define PATH_FIPS	"/proc/sys/crypto/fips_enabled"
#define PATH_LOCKDOWN	"/sys/kernel/security/lockdown"
#define SELINUX_PATH "/sys/fs/selinux"
#define SELINUX_STATUS_PATH (SELINUX_PATH "/enforce")

#if defined(__powerpc64__) || defined(__ppc64__)
# define SECUREBOOT_VAR "/proc/device-tree/ibm,secure-boot"
# define VAR_DATA_SIZE 4
#else
# define SECUREBOOT_VAR "/sys/firmware/efi/efivars/SecureBoot-8be4df61-93ca-11d2-aa0d-00e098032b8c"
# define VAR_DATA_SIZE 5
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mount.h>

#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_stdio.h"
#include "tst_security.h"
#include "tst_private.h"

int tst_lsm_enabled(const char *name)
{
	int fd;
	char *ptr;
	char data[BUFSIZ];

	if (access(LSM_SYS_FILE, F_OK))
		tst_brk(TCONF, "%s file is not present", LSM_SYS_FILE);

	fd = SAFE_OPEN(LSM_SYS_FILE, O_RDONLY);
	SAFE_READ(0, fd, data, BUFSIZ);
	SAFE_CLOSE(fd);

	ptr = strtok(data, ",");
	while (ptr != NULL) {
		if (!strcmp(ptr, name)) {
			tst_res(TINFO, "%s is enabled", name);
			return 1;
		}

		ptr = strtok(NULL, ",");
	}

	return 0;
}

int tst_fips_enabled(void)
{
	int fips = 0;

	if (access(PATH_FIPS, R_OK) == 0)
		SAFE_FILE_SCANF(PATH_FIPS, "%d", &fips);

	tst_res(TINFO, "FIPS: %s", fips ? "on" : "off");

	return fips;
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

int tst_selinux_enforcing(void)
{
	int res = 0;

	if (access(SELINUX_STATUS_PATH, F_OK) == 0)
		SAFE_FILE_SCANF(SELINUX_STATUS_PATH, "%d", &res);

	tst_res(TINFO, "SELinux enforcing: %s", res ? "on" : "off");

	return res;
}
