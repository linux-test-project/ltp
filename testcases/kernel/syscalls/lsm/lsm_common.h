/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LSM_GET_SELF_ATTR_H
#define LSM_GET_SELF_ATTR_H

#include "tst_test.h"
#include "lapi/lsm.h"

#define LSM_SYS_FILE "/sys/kernel/security/lsm"

static inline struct lsm_ctx *next_ctx(struct lsm_ctx *tctx)
{
	return (struct lsm_ctx *)((char *)tctx + sizeof(*tctx) + tctx->ctx_len);
}

static inline void read_proc_attr(const char *attr, char *val, const size_t size)
{
	int fd;
	char *ptr;
	char path[BUFSIZ];

	memset(val, 0, size);
	memset(path, 0, BUFSIZ);

	snprintf(path, BUFSIZ, "/proc/self/attr/%s", attr);

	tst_res(TINFO, "Reading %s", path);

	fd = SAFE_OPEN(path, O_RDONLY);

	if (read(fd, val, size) > 0) {
		ptr = strchr(val, '\n');
		if (ptr)
			*ptr = '\0';
	}

	SAFE_CLOSE(fd);
}

static inline int verify_enabled_lsm(const char *name)
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

static inline uint32_t count_supported_attr_current(void)
{
	uint32_t lsm_count = 0;

	if (verify_enabled_lsm("selinux"))
		lsm_count++;

	if (verify_enabled_lsm("apparmor"))
		lsm_count++;

	if (verify_enabled_lsm("smack"))
		lsm_count++;

	return lsm_count;
}

static inline uint32_t verify_supported_attr_current(void)
{
	uint32_t lsm_count;

	lsm_count = count_supported_attr_current();

	if (!lsm_count)
		tst_brk(TCONF, "LSM_ATTR_CURRENT is not supported by any LSM");

	return lsm_count;
}
#endif
