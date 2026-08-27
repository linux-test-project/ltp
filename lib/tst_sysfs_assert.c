// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_sysfs_assert.h"

static void build_path(char *path, size_t size, const char *fmt, va_list ap)
{
	vsnprintf(path, size, fmt, ap);
}

static void build_suffix_paths(char *path1, char *path2, size_t size,
			       const char *lo_suffix, const char *hi_suffix,
			       const char *fmt, va_list ap)
{
	char prefix[PATH_MAX];

	build_path(prefix, sizeof(prefix), fmt, ap);

	snprintf(path1, size, "%s%s", prefix, lo_suffix);
	snprintf(path2, size, "%s%s", prefix, hi_suffix);
}

static FILE *try_fopen(const char *file, const int lineno, const char *path)
{
	FILE *f = fopen(path, "r");

	if (f)
		return f;

	if (errno == ENOENT)
		tst_res_(file, lineno, TCONF, "%s does not exist", path);
	else
		tst_res_(file, lineno, TBROK | TERRNO, "Failed to open %s", path);

	return NULL;
}

static void range_check(const char *file, const int lineno,
			const char *path, long long min, long long max)
{
	long long val;

	if (access(path, F_OK)) {
		tst_res_(file, lineno, TCONF, "%s does not exist", path);
		return;
	}

	if (file_scanf(file, lineno, path, "%lld", &val)) {
		tst_res_(file, lineno, TFAIL, "%s does not contain a number", path);
		return;
	}

	if (val < min || val > max) {
		tst_res_(file, lineno, TFAIL,
			 "%s = %lld out of range [%lld, %lld]", path, val, min, max);
		return;
	}

	tst_res_(file, lineno, TPASS,
		 "%s = %lld in range [%lld, %lld]", path, val, min, max);
}

void tst_sysfs_assert_rangell(const char *file, const int lineno,
			      long long min, long long max, const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	range_check(file, lineno, path, min, max);
}

void tst_sysfs_assert_rangell_silent(const char *file, const int lineno,
				     long long min, long long max, const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	if (access(path, F_OK))
		return;

	range_check(file, lineno, path, min, max);
}

static void range_check_u(const char *file, const int lineno,
			  const char *path, unsigned long long min,
			  unsigned long long max)
{
	unsigned long long val;

	if (access(path, F_OK)) {
		tst_res_(file, lineno, TCONF, "%s does not exist", path);
		return;
	}

	if (file_scanf(file, lineno, path, "%llu", &val)) {
		tst_res_(file, lineno, TFAIL, "%s does not contain a number", path);
		return;
	}

	if (val < min || val > max) {
		tst_res_(file, lineno, TFAIL,
			 "%s = %llu out of range [%llu, %llu]", path, val, min, max);
		return;
	}

	tst_res_(file, lineno, TPASS,
		 "%s = %llu in range [%llu, %llu]", path, val, min, max);
}

void tst_sysfs_assert_rangeuu(const char *file, const int lineno,
			      unsigned long long min, unsigned long long max,
			      const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	range_check_u(file, lineno, path, min, max);
}

void tst_sysfs_assert_rangeuu_silent(const char *file, const int lineno,
				     unsigned long long min,
				     unsigned long long max,
				     const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	if (access(path, F_OK))
		return;

	range_check_u(file, lineno, path, min, max);
}

static void range_check_f(const char *file, const int lineno, long long min,
			  const char *val_path, const char *max_path)
{
	long long max;

	if (access(max_path, F_OK)) {
		tst_res_(file, lineno, TCONF, "%s does not exist", max_path);
		return;
	}

	if (file_scanf(file, lineno, max_path, "%lld", &max)) {
		tst_res_(file, lineno, TFAIL, "%s does not contain a number",
			 max_path);
		return;
	}

	range_check(file, lineno, val_path, min, max);
}

void tst_sysfs_assert_rangelf(const char *file, const int lineno,
			      long long min, const char *val_suffix,
			      const char *max_suffix,
			      const char *fmt, ...)
{
	char val_path[PATH_MAX], max_path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_suffix_paths(val_path, max_path, sizeof(val_path), val_suffix,
			   max_suffix, fmt, ap);
	va_end(ap);

	range_check_f(file, lineno, min, val_path, max_path);
}

static void bool_check(const char *file, const int lineno, const char *path)
{
	long val;

	if (access(path, F_OK)) {
		tst_res_(file, lineno, TCONF, "%s does not exist", path);
		return;
	}

	if (file_scanf(file, lineno, path, "%ld", &val)) {
		tst_res_(file, lineno, TFAIL, "%s does not contain a number", path);
		return;
	}

	if (val != 0 && val != 1) {
		tst_res_(file, lineno, TFAIL, "%s = %ld is not boolean", path, val);
		return;
	}

	tst_res_(file, lineno, TPASS, "%s = %ld is boolean", path, val);
}

void tst_sysfs_assert_bool(const char *file, const int lineno,
			   const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	bool_check(file, lineno, path);
}

void tst_sysfs_assert_bool_silent(const char *file, const int lineno,
				  const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	if (access(path, F_OK))
		return;

	bool_check(file, lineno, path);
}

static void pow2_check(const char *file, const int lineno, const char *path)
{
	long val;

	if (access(path, F_OK)) {
		tst_res_(file, lineno, TCONF, "%s does not exist", path);
		return;
	}

	if (file_scanf(file, lineno, path, "%ld", &val)) {
		tst_res_(file, lineno, TFAIL, "%s does not contain a number", path);
		return;
	}

	if (val <= 0 || (val & (val - 1))) {
		tst_res_(file, lineno, TFAIL,
			 "%s = %ld is not a power of two", path, val);
		return;
	}

	tst_res_(file, lineno, TPASS, "%s = %ld is a power of two", path, val);
}

void tst_sysfs_assert_pow2(const char *file, const int lineno,
			   const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	pow2_check(file, lineno, path);
}

static int is_allowed(const char *const allowed[], const char *val)
{
	const char *const *a;

	for (a = allowed; *a; a++) {
		if (!strcmp(*a, val))
			return 1;
	}

	return 0;
}

static void tokens_check(const char *file, const int lineno,
			 const char *path, const char *const allowed[],
			 char *sel, size_t sel_size, int sel_expected,
			 const char *find)
{
	FILE *f;
	char *tok = NULL;
	size_t tok_size = 0;
	ssize_t len;
	int nselected = 0;
	int ntokens = 0;
	int bad = 0;
	int found = 0;

	f = try_fopen(file, lineno, path);
	if (!f)
		return;

	while ((len = getdelim(&tok, &tok_size, ' ', f)) != -1) {
		int selected = 0;
		const char *name = tok;

		/* Strip the trailing ' ' or '\n' left by getdelim(). */
		while (len > 0 && (tok[len - 1] == ' ' || tok[len - 1] == '\n'))
			tok[--len] = '\0';

		/* Skip empty tokens, e.g. from runs of consecutive spaces. */
		if (len == 0)
			continue;

		if (sel_expected && len >= 2 && tok[0] == '[' && tok[len - 1] == ']') {
			selected = 1;
			tok[len - 1] = '\0';
			name = tok + 1;
		}

		ntokens++;

		if (selected) {
			nselected++;
			if (sel && sel_size) {
				strncpy(sel, name, sel_size - 1);
				sel[sel_size - 1] = '\0';
			}
		}

		if (find && !strcmp(find, name))
			found = 1;

		if (allowed && !is_allowed(allowed, name)) {
			tst_res_(file, lineno, TFAIL,
				 "%s contains unexpected token '%s'",
				 path, name);
			bad = 1;
		}
	}

	fclose(f);

	if (!ntokens) {
		tst_res_(file, lineno, TFAIL, "%s is empty", path);
		goto out;
	}

	if (sel_expected && nselected != 1) {
		tst_res_(file, lineno, TFAIL,
			 "%s has %d selected tokens, expected exactly 1",
			 path, nselected);
		goto out;
	}

	if (bad)
		goto out;

	if (find && !found) {
		tst_res_(file, lineno, TFAIL,
			 "%s does not contain token '%s'", path, find);
		goto out;
	}

	if (sel_expected) {
		tst_res_(file, lineno, TPASS,
			 "%s has valid choices with single selected", path);
	} else if (find) {
		tst_res_(file, lineno, TPASS,
			 "%s contains token '%s'", path, find);
	} else {
		tst_res_(file, lineno, TPASS,
			 "%s lists only known tokens", path);
	}
out:
	free(tok);
}

void tst_sysfs_assert_choice(const char *file, const int lineno,
			     const char *const allowed[], char *sel,
			     size_t sel_size, const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	tokens_check(file, lineno, path, allowed, sel, sel_size, 1, NULL);
}

void tst_sysfs_assert_tokens(const char *file, const int lineno,
			     const char *const allowed[], const char *find,
			     const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	tokens_check(file, lineno, path, allowed, NULL, 0, 0, find);
}

void tst_sysfs_assert_oneof(const char *file, const int lineno,
			    const char *const allowed[], const char *fmt, ...)
{
	char path[PATH_MAX];
	char val[256];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	if (access(path, F_OK)) {
		tst_res_(file, lineno, TCONF, "%s does not exist", path);
		return;
	}

	if (file_scanf(file, lineno, path, "%255s", val)) {
		tst_res_(file, lineno, TFAIL, "Failed to read %s", path);
		return;
	}

	if (is_allowed(allowed, val))
		tst_res_(file, lineno, TPASS, "%s = '%s' is valid", path, val);
	else
		tst_res_(file, lineno, TFAIL,
			 "%s = '%s' is not an expected value", path, val);
}

static int parse_range_token(char *tok, int *a, int *b)
{
	char *dash;
	int ret = 0;

	dash = strchr(tok, '-');
	if (!dash) {
		if (tst_parse_int(tok, a, 0, INT_MAX))
			return -1;

		*b = *a;
		return 0;
	}

	*dash = '\0';

	if (tst_parse_int(tok, a, 0, INT_MAX) ||
	    tst_parse_int(dash + 1, b, *a, INT_MAX)) {
		ret = -1;
	}

	*dash = '-';

	return ret;
}

static int parse_list(const char *file, const int lineno, const char *path,
		      unsigned char *map, int *count, int *max_id)
{
	FILE *f;
	char *tok = NULL;
	size_t tok_size = 0;
	ssize_t len;
	int total = 0;
	int maxid = -1;

	f = try_fopen(file, lineno, path);
	if (!f)
		return -1;

	while ((len = getdelim(&tok, &tok_size, ',', f)) != -1) {
		int a, b, i;

		/* Strip the trailing ',' or '\n' left by getdelim(). */
		while (len > 0 && (tok[len - 1] == ',' || tok[len - 1] == '\n'))
			tok[--len] = '\0';

		/*
		 * Skip empty tokens, e.g. a lone trailing newline on an
		 * otherwise empty file (all CPUs online -> empty offline
		 * cpulist).
		 */
		if (len == 0)
			continue;

		if (parse_range_token(tok, &a, &b)) {
			tst_res_(file, lineno, TFAIL,
				 "%s: invalid list token '%s'", path, tok);
			free(tok);
			fclose(f);
			return -1;
		}

		for (i = a; i <= b; i++) {
			total++;
			if (i > maxid)
				maxid = i;
			if (map)
				map[i / 8] |= 1 << (i % 8);
		}
	}

	free(tok);
	fclose(f);

	if (count)
		*count = total;
	if (max_id)
		*max_id = maxid;

	return 0;
}

int tst_sysfs_assert_parse_list(const char *file, const int lineno,
				int *count, int *max_id, const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	return parse_list(file, lineno, path, NULL, count, max_id);
}

static unsigned char *read_list_map(const char *file, const int lineno,
				    const char *path, int *map_bytes)
{
	unsigned char *map;
	int max_id, bytes;

	/* First pass: only to find the highest id so we can size the bitmap. */
	if (parse_list(file, lineno, path, NULL, NULL, &max_id))
		return NULL;

	bytes = max_id < 0 ? 1 : max_id / 8 + 1;
	map = SAFE_MALLOC(bytes);
	memset(map, 0, bytes);

	/* Second pass: fill the bitmap. */
	if (parse_list(file, lineno, path, map, NULL, NULL)) {
		free(map);
		return NULL;
	}

	*map_bytes = bytes;
	return map;
}

void tst_sysfs_assert_list_subset(const char *file, const int lineno,
				  const char *sub_path,
				  const char *super_path)
{
	unsigned char *sub = NULL, *super = NULL;
	int sub_bytes, super_bytes, i;

	sub = read_list_map(file, lineno, sub_path, &sub_bytes);
	if (!sub)
		return;

	super = read_list_map(file, lineno, super_path, &super_bytes);
	if (!super) {
		free(sub);
		return;
	}

	for (i = 0; i < sub_bytes; i++) {
		unsigned char super_byte = i < super_bytes ? super[i] : 0;

		if (sub[i] & ~super_byte) {
			tst_res_(file, lineno, TFAIL,
				 "%s is not a subset of %s",
				 sub_path, super_path);
			free(sub);
			free(super);
			return;
		}
	}

	free(sub);
	free(super);

	tst_res_(file, lineno, TPASS,
		 "%s is a subset of %s", sub_path, super_path);
}

void tst_sysfs_assert_list_contains(const char *file, const int lineno,
				    int id, const char *fmt, ...)
{
	char path[PATH_MAX];
	unsigned char *map;
	int map_bytes;
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	map = read_list_map(file, lineno, path, &map_bytes);
	if (!map)
		return;

	if (id >= 0 && id / 8 < map_bytes && (map[id / 8] & (1 << (id % 8)))) {
		tst_res_(file, lineno, TPASS, "%s contains %d", path, id);
	} else {
		tst_res_(file, lineno, TFAIL, "%s does not contain %d", path,
			 id);
	}

	free(map);
}

void tst_sysfs_assert_cmp(const char *file, const int lineno,
			  const char *path1, enum tst_sysfs_cmp op,
			  const char *path2)
{
	long val1, val2;
	int res;
	const char *opstr;

	if (access(path1, F_OK)) {
		tst_res_(file, lineno, TCONF, "%s does not exist", path1);
		return;
	}

	if (access(path2, F_OK)) {
		tst_res_(file, lineno, TCONF, "%s does not exist", path2);
		return;
	}

	if (file_scanf(file, lineno, path1, "%ld", &val1)) {
		tst_res_(file, lineno, TFAIL, "%s does not contain a number", path1);
		return;
	}

	if (file_scanf(file, lineno, path2, "%ld", &val2)) {
		tst_res_(file, lineno, TFAIL, "%s does not contain a number", path2);
		return;
	}

	switch (op) {
	case TST_SYSFS_CMP_EQ:
		res = val1 == val2;
		opstr = "==";
		break;
	case TST_SYSFS_CMP_LT:
		res = val1 < val2;
		opstr = "<";
		break;
	case TST_SYSFS_CMP_LE:
		res = val1 <= val2;
		opstr = "<=";
		break;
	default:
		tst_res_(file, lineno, TBROK, "Invalid comparison operator %d", op);
		return;
	}

	tst_res_(file, lineno, res ? TPASS : TFAIL,
		 "%s (%ld) %s %s (%ld)",
		 path1, val1, opstr, path2, val2);
}

void tst_sysfs_assert_cmp_silent(const char *file, const int lineno,
				 const char *path1, enum tst_sysfs_cmp op,
				 const char *path2)
{
	if (access(path1, F_OK) || access(path2, F_OK))
		return;

	tst_sysfs_assert_cmp(file, lineno, path1, op, path2);
}

void tst_sysfs_assert_cmp_suffix(const char *file, const int lineno,
				 enum tst_sysfs_cmp op, const char *lo_suffix,
				 const char *hi_suffix, const char *fmt, ...)
{
	char path1[PATH_MAX], path2[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_suffix_paths(path1, path2, sizeof(path1), lo_suffix, hi_suffix,
			   fmt, ap);
	va_end(ap);

	tst_sysfs_assert_cmp(file, lineno, path1, op, path2);
}

void tst_sysfs_assert_cmp_suffix_silent(const char *file, const int lineno,
					enum tst_sysfs_cmp op,
					const char *lo_suffix,
					const char *hi_suffix,
					const char *fmt, ...)
{
	char path1[PATH_MAX], path2[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_suffix_paths(path1, path2, sizeof(path1), lo_suffix, hi_suffix,
			   fmt, ap);
	va_end(ap);

	tst_sysfs_assert_cmp_silent(file, lineno, path1, op, path2);
}

int tst_sysfs_exists(const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	return !access(path, F_OK);
}

DIR *tst_sysfs_try_opendir(const char *file, const int lineno,
			   const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	if (access(path, F_OK))
		tst_brk_(file, lineno, TCONF, "%s does not exist", path);

	return safe_opendir(file, lineno, NULL, path);
}

int tst_sysfs_read_str(const char *file, const int lineno, char *buf,
		       size_t buf_size, const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	if (access(path, F_OK))
		return 0;

	safe_file_read_str(file, lineno, path, buf, buf_size);
	buf[strcspn(buf, "\n")] = '\0';

	return 1;
}

void tst_sysfs_exp_eq_li(const char *file, const int lineno, long val,
			 const char *fmt, ...)
{
	char path[PATH_MAX];
	va_list ap;
	long read_val;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	if (access(path, F_OK)) {
		tst_res_(file, lineno, TCONF, "%s does not exist", path);
		return;
	}

	if (file_scanf(file, lineno, path, "%ld", &read_val)) {
		tst_res_(file, lineno, TFAIL, "%s does not contain a number", path);
		return;
	}

	tst_res_(file, lineno, read_val == val ? TPASS : TFAIL,
		 "%s (%ld) == %ld", path, read_val, val);
}

void tst_sysfs_exp_eq_str(const char *file, const int lineno, const char *val,
			  const char *fmt, ...)
{
	char path[PATH_MAX];
	char read_val[256];
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	if (access(path, F_OK)) {
		tst_res_(file, lineno, TCONF, "%s does not exist", path);
		return;
	}

	if (file_scanf(file, lineno, path, "%255s", read_val)) {
		tst_res_(file, lineno, TFAIL, "Failed to read %s", path);
		return;
	}

	tst_res_(file, lineno, !strcmp(read_val, val) ? TPASS : TFAIL,
		 "%s ('%s') == '%s'", path, read_val, val);
}

long long tst_sysfs_read_lli(const char *file, const int lineno,
			     const char *fmt, ...)
{
	char path[PATH_MAX];
	long long val;
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	safe_file_scanf(file, lineno, NULL, path, "%lld", &val);

	tst_res(TDEBUG, "Read '%s' = %lli", path, val);

	return val;
}

long tst_sysfs_read_li(const char *file, const int lineno,
		       const char *fmt, ...)
{
	char path[PATH_MAX];
	long val;
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	safe_file_scanf(file, lineno, NULL, path, "%ld", &val);

	tst_res(TDEBUG, "Read '%s' = %li", path, val);

	return val;
}

unsigned long tst_sysfs_read_lx(const char *file, const int lineno,
				const char *fmt, ...)
{
	char path[PATH_MAX];
	unsigned long val;
	va_list ap;

	va_start(ap, fmt);
	build_path(path, sizeof(path), fmt, ap);
	va_end(ap);

	safe_file_scanf(file, lineno, NULL, path, "%lx", &val);

	tst_res(TDEBUG, "Read '%s' = 0x%lx", path, val);

	return val;
}
