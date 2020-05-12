// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/utsname.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_kconfig.h"

static const char *kconfig_path(char *path_buf, size_t path_buf_len)
{
	const char *path = getenv("KCONFIG_PATH");
	struct utsname un;

	if (path) {
		if (!access(path, F_OK))
			return path;

		tst_res(TWARN, "KCONFIG_PATH='%s' does not exist", path);
	}

	if (!access("/proc/config.gz", F_OK))
		return "/proc/config.gz";

	uname(&un);

	/* Debian and derivatives */
	snprintf(path_buf, path_buf_len, "/boot/config-%s", un.release);

	if (!access(path_buf, F_OK))
		return path_buf;

	/* Clear Linux */
	snprintf(path_buf, path_buf_len, "/lib/kernel/config-%s", un.release);

	if (!access(path_buf, F_OK))
		return path_buf;

	tst_res(TINFO, "Couldn't locate kernel config!");

	return NULL;
}

static char is_gzip;

static FILE *open_kconfig(void)
{
	FILE *fp;
	char buf[1064];
	char path_buf[1024];
	const char *path = kconfig_path(path_buf, sizeof(path_buf));

	if (!path)
		return NULL;

	tst_res(TINFO, "Parsing kernel config '%s'", path);

	is_gzip = !!strstr(path, ".gz");

	if (is_gzip) {
		snprintf(buf, sizeof(buf), "zcat '%s'", path);
		fp = popen(buf, "r");
	} else {
		fp = fopen(path, "r");
	}

	if (!fp)
		tst_brk(TBROK | TERRNO, "Failed to open '%s'", path);

	return fp;
}

static void close_kconfig(FILE *fp)
{
	if (is_gzip)
		pclose(fp);
	else
		fclose(fp);
}

struct match {
	/* match len, string length up to \0 or = */
	size_t len;
	/* if set part of conf string after = */
	const char *val;
	/* if set the config option was matched already */
	int match;
};

static int is_set(const char *str, const char *val)
{
	size_t vlen = strlen(val);

	while (isspace(*str))
		str++;

	if (strncmp(str, val, vlen))
		return 0;

	switch (str[vlen]) {
	case ' ':
	case '\n':
	case '\0':
		return 1;
	break;
	default:
		return 0;
	}
}

static inline int match(struct match *match, const char *conf,
                        struct tst_kconfig_res *result, const char *line)
{
	if (match->match)
		return 0;

	const char *cfg = strstr(line, "CONFIG_");

	if (!cfg)
		return 0;

	if (strncmp(cfg, conf, match->len))
		return 0;

	const char *val = &cfg[match->len];

	switch (cfg[match->len]) {
	case '=':
		break;
	case ' ':
		if (is_set(val, "is not set")) {
			result->match = 'n';
			goto match;
		}
	/* fall through */
	default:
		return 0;
	}

	if (is_set(val, "=y")) {
		result->match = 'y';
		goto match;
	}

	if (is_set(val, "=m")) {
		result->match = 'm';
		goto match;
	}

	result->match = 'v';
	result->value = strndup(val+1, strlen(val)-2);

match:
	match->match = 1;
	return 1;
}

void tst_kconfig_read(const char *const *kconfigs,
                      struct tst_kconfig_res results[], size_t cnt)
{
	struct match matches[cnt];
	FILE *fp;
	unsigned int i, j;
	char buf[1024];

	for (i = 0; i < cnt; i++) {
		const char *val = strchr(kconfigs[i], '=');

		if (strncmp("CONFIG_", kconfigs[i], 7))
			tst_brk(TBROK, "Invalid config string '%s'", kconfigs[i]);

		matches[i].match = 0;
		matches[i].len = strlen(kconfigs[i]);

		if (val) {
			matches[i].val = val + 1;
			matches[i].len -= strlen(val);
		}

		results[i].match = 0;
		results[i].value = NULL;
	}

	fp = open_kconfig();
	if (!fp)
		tst_brk(TBROK, "Cannot parse kernel .config");

	while (fgets(buf, sizeof(buf), fp)) {
		for (i = 0; i < cnt; i++) {
			if (match(&matches[i], kconfigs[i], &results[i], buf)) {
				for (j = 0; j < cnt; j++) {
					if (matches[j].match)
						break;
				}

				if (j == cnt)
					goto exit;
			}
		}

	}

exit:
	close_kconfig(fp);
}

static size_t array_len(const char *const kconfigs[])
{
	size_t i = 0;

	while (kconfigs[++i]);

	return i;
}

static int compare_res(struct tst_kconfig_res *res, const char *kconfig,
                       char match, const char *val)
{
	if (res->match != match) {
		tst_res(TINFO, "Needs kernel %s, have %c", kconfig, res->match);
		return 1;
	}

	if (match != 'v')
		return 0;

	if (strcmp(res->value, val)) {
		tst_res(TINFO, "Needs kernel %s, have %s", kconfig, res->value);
		return 1;
	}

	return 0;
}

void tst_kconfig_check(const char *const kconfigs[])
{
	size_t cnt = array_len(kconfigs);
	struct tst_kconfig_res results[cnt];
	unsigned int i;
	int abort_test = 0;

	tst_kconfig_read(kconfigs, results, cnt);

	for (i = 0; i < cnt; i++) {
		if (results[i].match == 0) {
			tst_res(TINFO, "Missing kernel %s", kconfigs[i]);
			abort_test = 1;
			continue;
		}

		if (results[i].match == 'n') {
			tst_res(TINFO, "Kernel %s is not set", kconfigs[i]);
			abort_test = 1;
			continue;
		}

		const char *val = strchr(kconfigs[i], '=');

		if (val) {
			char match = 'v';
			val++;

			if (!strcmp(val, "y"))
				match = 'y';

			if (!strcmp(val, "m"))
				match = 'm';

			if (compare_res(&results[i], kconfigs[i], match, val))
				abort_test = 1;

		}

		free(results[i].value);
	}

	if (abort_test)
		tst_brk(TCONF, "Aborting due to unsuitable kernel config, see above!");
}
