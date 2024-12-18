// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Jan Stancek <jstancek@redhat.com>
 */

#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_sys_conf.h"

struct tst_sys_conf {
	char path[PATH_MAX];
	char value[PATH_MAX];
	struct tst_sys_conf *next;
};

static struct tst_sys_conf *save_restore_data;

static void print_error(const int lineno, int info_only, const char *err,
	const char *path)
{
	if (info_only)
		tst_res_(__FILE__, lineno, TINFO | TERRNO, err, path);
	else
		tst_brk_(__FILE__, lineno, TBROK | TERRNO, err, path);
}

void tst_sys_conf_dump(void)
{
	struct tst_sys_conf *i;

	for (i = save_restore_data; i; i = i->next)
		tst_res(TINFO, "%s = %s", i->path, i->value);
}

void tst_sys_conf_save_str(const char *path, const char *value)
{
	struct tst_sys_conf *n = SAFE_MALLOC(sizeof(*n));

	strncpy(n->path, path, sizeof(n->path)-1);
	strncpy(n->value, value, sizeof(n->value)-1);

	n->path[sizeof(n->path) - 1] = 0;
	n->value[sizeof(n->value) - 1] = 0;

	n->next = save_restore_data;
	save_restore_data = n;
}

int tst_sys_conf_save(const struct tst_path_val *conf)
{
	char line[PATH_MAX];
	int ttype, iret;
	FILE *fp;
	void *ret;

	if (!conf || !conf->path)
		tst_brk(TBROK, "path is empty");

	if (access(conf->path, F_OK) != 0) {
		if (conf->flags & TST_SR_SKIP_MISSING) {
			tst_res(TINFO | TERRNO, "Path not found: %s",
				conf->path);
			return 1;
		}

		ttype = (conf->flags & TST_SR_TBROK_MISSING) ? TBROK : TCONF;
		tst_brk(ttype | TERRNO, "Path not found: %s", conf->path);
	}

	if (access(conf->path, W_OK) != 0) {
		if (conf->flags & TST_SR_SKIP_RO) {
			tst_res(TINFO | TERRNO, "Path is not writable: %s",
				conf->path);
			return 1;
		}

		ttype = (conf->flags & TST_SR_TBROK_RO) ? TBROK : TCONF;
		tst_brk(ttype | TERRNO, "Path is not writable: %s", conf->path);
	}

	fp = fopen(conf->path, "r");

	if (fp == NULL) {
		print_error(__LINE__, conf->flags & TST_SR_IGNORE_ERR,
			"Failed to open '%s' for reading", conf->path);
		return 1;
	}

	ret = fgets(line, sizeof(line), fp);
	fclose(fp);

	if (ret == NULL) {
		if (conf->flags & TST_SR_IGNORE_ERR)
			return 1;

		tst_brk(TBROK | TERRNO, "Failed to read anything from '%s'",
			conf->path);
	}

	tst_sys_conf_save_str(conf->path, line);

	if (!conf->val)
		return 0;

	fp = fopen(conf->path, "w");

	if (fp == NULL) {
		print_error(__LINE__, conf->flags & TST_SR_IGNORE_ERR,
			"Failed to open '%s' for writing", conf->path);
		return 0;
	}

	iret = fputs(conf->val, fp);

	if (iret < 0) {
		print_error(__LINE__, conf->flags & TST_SR_IGNORE_ERR,
			"Failed to write into '%s'", conf->path);
	}

	iret = fclose(fp);

	if (iret < 0) {
		print_error(__LINE__, conf->flags & TST_SR_IGNORE_ERR,
			"Failed to close '%s'", conf->path);
	}

	return 0;
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

int tst_read_bool_sys_param(const char *filename)
{
	char buf[PATH_MAX];
	int i, fd, ret;

	fd = open(filename, O_RDONLY);

	if (fd < 0)
		return -1;

	ret = read(fd, buf, PATH_MAX - 1);
	SAFE_CLOSE(fd);

	if (ret < 1)
		return -1;

	buf[ret] = '\0';

	for (i = 0; buf[i] && !isspace(buf[i]); i++)
		;

	buf[i] = '\0';

	if (isdigit(buf[0])) {
		tst_parse_int(buf, &ret, INT_MIN, INT_MAX);
		return ret;
	}

	if (!strcasecmp(buf, "N"))
		return 0;

	/* Assume that any other value than 0 or N means the param is enabled */
	return 1;
}

long tst_sys_conf_long_get_(const char *file, const int lineno,
			    const char *path)
{
	long ret;

	safe_file_scanf(file, lineno, NULL, path, "%ld", &ret);

	return ret;
}

void tst_sys_conf_long_set_(const char *file, const int lineno,
			    const char *path, long val, int check)
{
	tst_res_(file, lineno, TINFO, "Setting %s to %ld", path, val);

	safe_file_printf(file, lineno, NULL, path, "%ld", val);

	if (check) {
		long read_val;

		safe_file_scanf(file, lineno, NULL, path, "%ld", &read_val);

		if (val != read_val)
			tst_brk_(file, lineno, TBROK,
				 "Wrote %ld to %s but read back %ld",
				 val, path, read_val);
	}
}
