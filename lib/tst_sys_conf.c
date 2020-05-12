// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Jan Stancek <jstancek@redhat.com>
 */

#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_sys_conf.h"

static struct tst_sys_conf *save_restore_data;

void tst_sys_conf_dump(void)
{
	struct tst_sys_conf *i;

	for (i = save_restore_data; i; i = i->next)
		tst_res(TINFO, "%s = %s", i->path, i->value);
}

int tst_sys_conf_save_str(const char *path, const char *value)
{
	struct tst_sys_conf *n = SAFE_MALLOC(sizeof(*n));

	strncpy(n->path, path, sizeof(n->path)-1);
	strncpy(n->value, value, sizeof(n->value)-1);

	n->path[sizeof(n->path) - 1] = 0;
	n->value[sizeof(n->value) - 1] = 0;

	n->next = save_restore_data;
	save_restore_data = n;

	return 0;
}

int tst_sys_conf_save(const char *path)
{
	char line[PATH_MAX];
	FILE *fp;
	void *ret;
	char flag;

	if (!path)
		tst_brk(TBROK, "path is empty");

	flag = path[0];
	if (flag  == '?' || flag == '!')
		path++;

	if (access(path, F_OK) != 0) {
		switch (flag) {
		case '?':
			tst_res(TINFO, "Path not found: '%s'", path);
			break;
		case '!':
			tst_brk(TBROK|TERRNO, "Path not found: '%s'", path);
			break;
		default:
			tst_brk(TCONF|TERRNO, "Path not found: '%s'", path);
		}
		return 1;
	}

	fp = fopen(path, "r");
	if (fp == NULL) {
		if (flag == '?')
			return 1;

		tst_brk(TBROK | TERRNO, "Failed to open FILE '%s' for reading",
			path);
		return 1;
	}

	ret = fgets(line, sizeof(line), fp);
	fclose(fp);

	if (ret == NULL) {
		if (flag == '?')
			return 1;

		tst_brk(TBROK | TERRNO, "Failed to read anything from '%s'",
			path);
	}

	return tst_sys_conf_save_str(path, line);
}

void tst_sys_conf_restore(int verbose)
{
	struct tst_sys_conf *i;

	for (i = save_restore_data; i; i = i->next) {
		if (verbose) {
			tst_res(TINFO, "Restoring conf.: %s -> %s\n",
				i->path, i->value);
		}
		FILE_PRINTF(i->path, "%s", i->value);
	}
}

