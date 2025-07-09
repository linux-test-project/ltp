/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LSM_GET_SELF_ATTR_H
#define LSM_GET_SELF_ATTR_H

#include "tst_test.h"
#include "lapi/lsm.h"

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

static inline uint32_t count_supported_attr_current(void)
{
	uint32_t lsm_count = 0;

	if (tst_lsm_enabled("selinux"))
		lsm_count++;

	if (tst_lsm_enabled("apparmor"))
		lsm_count++;

	if (tst_lsm_enabled("smack"))
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
