/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 */

#include "../libclone/libclone.h"
#include "test.h"
#include "safe_macros.h"
#include <stdbool.h>

#define UID_MAP 0
#define GID_MAP 1

static int dummy_child(void *v)
{
	(void) v;
	return 0;
}

static int check_newuser(void)
{
	int pid, status;

	pid = do_clone_unshare_test(T_CLONE, CLONE_NEWUSER, dummy_child, NULL);
	if (pid == -1)
		tst_brkm(TCONF | TERRNO, NULL, "CLONE_NEWUSER not supported");
	SAFE_WAIT(NULL, &status);

	return 0;
}

LTP_ATTRIBUTE_UNUSED static int updatemap(int cpid, bool type, int idnum,
	int parentmappid, void (*cleanup)(void))
{
	char path[BUFSIZ];
	char content[BUFSIZ];
	int fd;

	if (type == UID_MAP)
		sprintf(path, "/proc/%d/uid_map", cpid);
	else if (type == GID_MAP)
		sprintf(path, "/proc/%d/gid_map", cpid);
	else
		tst_brkm(TBROK, cleanup, "invalid type parameter");

	sprintf(content, "%d %d 1", idnum, parentmappid);
	fd = SAFE_OPEN(cleanup, path, O_WRONLY, 0644);
	SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fd, content, strlen(content));
	SAFE_CLOSE(cleanup, fd);
	return 0;
}
