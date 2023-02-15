// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef COMMON_H
#define COMMON_H

#include "tst_test.h"

#define UID_MAP 0
#define GID_MAP 1

static inline void updatemap(int cpid, int type, int idnum, int parentmappid)
{
	char path[BUFSIZ];
	char content[BUFSIZ];
	int fd;

	switch (type) {
	case UID_MAP:
		sprintf(path, "/proc/%d/uid_map", cpid);
		break;
	case GID_MAP:
		sprintf(path, "/proc/%d/gid_map", cpid);
		break;
	default:
		tst_brk(TBROK, "invalid type parameter");
		break;
	}

	sprintf(content, "%d %d 1", idnum, parentmappid);

	fd = SAFE_OPEN(path, O_WRONLY, 0644);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, content, strlen(content));
	SAFE_CLOSE(fd);
}

#endif
