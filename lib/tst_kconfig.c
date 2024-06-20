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
#include "tst_private.h"
#include "tst_kconfig.h"
#include "tst_bool_expr.h"
#include "tst_safe_stdio.h"

static int kconfig_skip_check(void)
{
	char *skipped = getenv("KCONFIG_SKIP_CHECK");

	if (skipped) {
		tst_res(TINFO, "Skipping kernel config check as requested");
		return 1;
	}

	return 0;
}

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

	/* Common install module path */
	snprintf(path_buf, path_buf_len, "/lib/modules/%s/build/.config", un.release);

	if (!access(path_buf, F_OK))
		return path_buf;

	snprintf(path_buf, path_buf_len, "/lib/modules/%s/config", un.release);

	if (!access(path_buf, F_OK))
		return path_buf;

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

static inline int kconfig_parse_line(const char *line,
                                     struct tst_kconfig_var *vars,
                                     unsigned int vars_len)
{
	unsigned int i, var_len = 0;
	const char *var;
	int is_not_set = 0;

	while (isspace(*line))
		line++;

	if (*line == '#') {
		if (!strstr(line, "is not set"))
			return 0;

		is_not_set = 1;
	}

	var = strstr(line, "CONFIG_");

	if (!var)
		return 0;

	for (;;) {
		switch (var[var_len]) {
		case 'A' ... 'Z':
		case '0' ... '9':
		case '_':
			var_len++;
		break;
		default:
			goto out;
		break;
		}
	}

out:

	for (i = 0; i < vars_len; i++) {
		const char *val;
		unsigned int val_len = 0;

		if (vars[i].id_len != var_len)
			continue;

		if (strncmp(vars[i].id, var, var_len))
			continue;

		if (is_not_set) {
			vars[i].choice = 'n';
			return 1;
		}

		val = var + var_len;

		while (isspace(*val))
			val++;

		if (*val != '=')
			return 0;

		val++;

		while (isspace(*val))
			val++;

		while (!isspace(val[val_len]))
			val_len++;

		if (val_len == 1) {
			switch (val[0]) {
			case 'y':
				vars[i].choice = 'y';
				return 1;
			case 'm':
				vars[i].choice = 'm';
				return 1;
			}
		}

		vars[i].choice = 'v';
		vars[i].val = strndup(val, val_len);
	}

	return 0;
}

void tst_kconfig_read(struct tst_kconfig_var vars[], size_t vars_len)
{
	char line[128];
	unsigned int vars_found = 0;

	FILE *fp = open_kconfig();
	if (!fp)
		tst_brk(TBROK, "Cannot parse kernel .config");

	while (fgets(line, sizeof(line), fp)) {
		if (kconfig_parse_line(line, vars, vars_len))
			vars_found++;

		if (vars_found == vars_len)
			goto exit;
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

static const char *strnchr(const char *s, int c, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		if (s[i] == c)
			return s + i;
	}

	return NULL;
}

static inline unsigned int get_len(const char* kconfig, unsigned int len)
{
	const char *sep = strnchr(kconfig, '=', len);

	if (!sep)
		return len;

	return sep - kconfig;
}

static void print_err(FILE *f, const struct tst_expr_tok *var,
                      size_t spaces, const char *err)
{
	size_t i;

	for (i = 0; i < var->tok_len; i++)
		fputc(var->tok[i], f);

	fputc('\n', f);

	while (spaces--)
		fputc(' ', f);

	fprintf(f, "^\n%s\n\n", err);
}

static int validate_var(const struct tst_expr_tok *var)
{
	size_t i = 7;

	if (var->tok_len < 7 || strncmp(var->tok, "CONFIG_", 7)) {
		print_err(stderr, var, 0, "Expected CONFIG_ prefix");
		return 1;
	}

	while (var->tok[i]) {
		char c;

		if (i >= var->tok_len)
			return 0;

		c = var->tok[i];

		if ((c >= 'A' && c <= 'Z') || c == '_') {
			i++;
			continue;
		}

		if (c >= '0' && c <= '9') {
			i++;
			continue;
		}

		if (c == '=') {
			i++;
			break;
		}

		print_err(stderr, var, i, "Unexpected character in variable name");
		return 1;
	}

	if (i >= var->tok_len) {

		if (var->tok[i-1] == '=') {
			print_err(stderr, var, i, "Missing value");
			return -1;
		}

		return 0;
	}

	if (var->tok[i] == '"') {
		do {
			i++;
		} while (i < var->tok_len && var->tok[i] != '"');

		if (i < var->tok_len - 1) {
			print_err(stderr, var, i, "Garbage after a string");
			return 1;
		}

		if (var->tok[i] != '"') {
			print_err(stderr, var, i, "Untermianted string");
			return 1;
		}

		return 0;
	}

	do {
		i++;
	} while (i < var->tok_len && isalnum(var->tok[i]));

	if (i < var->tok_len) {
		print_err(stderr, var, i, "Invalid character in variable value");
		return 1;
	}

	return 0;
}

static int validate_vars(struct tst_expr *const exprs[], unsigned int expr_cnt)
{
	unsigned int i;
	const struct tst_expr_tok *j;
	unsigned int ret = 0;

	for (i = 0; i < expr_cnt; i++) {
		for (j = exprs[i]->rpn; j; j = j->next) {
			if (j->op == TST_OP_VAR)
				ret |= validate_var(j);
		}
	}

	return ret;
}


static inline unsigned int get_var_cnt(struct tst_expr *const exprs[],
                                       unsigned int expr_cnt)
{
	unsigned int i;
	const struct tst_expr_tok *j;
	unsigned int cnt = 0;

	for (i = 0; i < expr_cnt; i++) {
		for (j = exprs[i]->rpn; j; j = j->next) {
			if (j->op == TST_OP_VAR)
				cnt++;
		}
	}

	return cnt;
}

static const struct tst_kconfig_var *find_var(const struct tst_kconfig_var vars[],
                                        unsigned int var_cnt,
                                        const char *var)
{
	unsigned int i;

	for (i = 0; i < var_cnt; i++) {
		if (!strcmp(vars[i].id, var))
			return &vars[i];
	}

	return NULL;
}

/*
 * Fill in the kconfig variables array from the expressions. Also makes sure
 * that each variable is copied to the array exaclty once.
 */
static inline unsigned int populate_vars(struct tst_expr *exprs[],
                                         unsigned int expr_cnt,
                                    struct tst_kconfig_var vars[])
{
	unsigned int i;
	struct tst_expr_tok *j;
	unsigned int cnt = 0;

	for (i = 0; i < expr_cnt; i++) {
		for (j = exprs[i]->rpn; j; j = j->next) {
			const struct tst_kconfig_var *var;

			if (j->op != TST_OP_VAR)
				continue;

			vars[cnt].id_len = get_len(j->tok, j->tok_len);

			if (vars[cnt].id_len + 1 >= sizeof(vars[cnt].id))
				tst_brk(TBROK, "kconfig var id too long!");

			strncpy(vars[cnt].id, j->tok, vars[cnt].id_len);
			vars[cnt].id[vars[cnt].id_len] = 0;
			vars[cnt].choice = 0;
			vars[cnt].val = NULL;

			var = find_var(vars, cnt, vars[cnt].id);

			if (var)
				j->priv = var;
			else
				j->priv = &vars[cnt++];
		}
	}

	return cnt;
}

static int map(struct tst_expr_tok *expr)
{
	const struct tst_kconfig_var *var = expr->priv;

	if (var->choice == 0)
		return 0;

	const char *val = strnchr(expr->tok, '=', expr->tok_len);

	/* CONFIG_FOO evaluates to true if y or m */
	if (!val)
		return var->choice == 'y' || var->choice == 'm';

	val++;

	unsigned int len = expr->tok_len - (val - expr->tok);
	char choice = 'v';

	if (!strncmp(val, "n", len))
		choice = 'n';

	if (!strncmp(val, "y", len))
		choice = 'y';

	if (!strncmp(val, "m", len))
		choice = 'm';

	if (choice != 'v')
		return var->choice == choice;

	if (var->choice != 'v')
		return 0;

	if (strlen(var->val) != len)
		return 0;

	return !strncmp(val, var->val, len);
}

static void dump_vars(const struct tst_expr *expr)
{
	const struct tst_expr_tok *i;
	const struct tst_kconfig_var *var;

	tst_res(TINFO, "Variables:");

	for (i = expr->rpn; i; i = i->next) {
		if (i->op != TST_OP_VAR)
			continue;

		var = i->priv;

		if (!var->choice) {
			tst_res(TINFO, " %s Undefined", var->id);
			continue;
		}

		if (var->choice == 'v') {
			tst_res(TINFO, " %s=%s", var->id, var->val);
			continue;
		}

		tst_res(TINFO, " %s=%c", var->id, var->choice);
	}
}

int tst_kconfig_check(const char *const kconfigs[])
{
	size_t expr_cnt = array_len(kconfigs);
	struct tst_expr *exprs[expr_cnt];
	unsigned int i, var_cnt;
	int ret = 0;

	if (kconfig_skip_check())
		return 0;

	for (i = 0; i < expr_cnt; i++) {
		exprs[i] = tst_bool_expr_parse(kconfigs[i]);

		if (!exprs[i])
			tst_brk(TBROK, "Invalid kconfig expression!");
	}

	if (validate_vars(exprs, expr_cnt))
		tst_brk(TBROK, "Invalid kconfig variables!");

	var_cnt = get_var_cnt(exprs, expr_cnt);
	struct tst_kconfig_var vars[var_cnt];

	var_cnt = populate_vars(exprs, expr_cnt, vars);

	tst_kconfig_read(vars, var_cnt);

	for (i = 0; i < expr_cnt; i++) {
		int val = tst_bool_expr_eval(exprs[i], map);

		if (val != 1) {
			ret = 1;
			tst_res(TINFO, "Constraint '%s' not satisfied!", kconfigs[i]);
			dump_vars(exprs[i]);
		}

		tst_bool_expr_free(exprs[i]);
	}

	for (i = 0; i < var_cnt; i++) {
		if (vars[i].choice == 'v')
			free(vars[i].val);
	}

	return ret;
}

char tst_kconfig_get(const char *confname)
{
	struct tst_kconfig_var var;

	if (kconfig_skip_check())
		return 0;

	var.id_len = strlen(confname);

	if (var.id_len >= sizeof(var.id))
		tst_brk(TBROK, "Kconfig var name \"%s\" too long", confname);

	strcpy(var.id, confname);
	var.choice = 0;
	var.val = NULL;

	tst_kconfig_read(&var, 1);

	if (var.choice == 'v')
		free(var.val);

	return var.choice;
}

void tst_kcmdline_parse(struct tst_kcmdline_var params[], size_t params_len)
{
	char buf[256], line[1024];
	size_t b_pos = 0,l_pos =0, i;
	int var_id = -1;

	FILE *f = SAFE_FOPEN("/proc/cmdline", "r");

	if (fgets(line, sizeof(line), f) == NULL) {
		SAFE_FCLOSE(f);
		tst_brk(TBROK, "Failed to read /proc/cmdline");
	}

	for (l_pos = 0; line[l_pos] != '\0'; l_pos++) {
		char c = line[l_pos];

		switch (c) {
		case '=':
			buf[b_pos] = '\0';
			for (i = 0; i < params_len; i++) {
				if (strcmp(buf, params[i].key) == 0) {
					var_id = (int)i;
					params[i].found = true;
				}
			}

			b_pos = 0;
		break;
		case ' ':
		case '\n':
			buf[b_pos] = '\0';
			if (var_id >= 0 && var_id < (int)params_len)
				strcpy(params[var_id].value, buf);

			var_id = -1;
			b_pos = 0;
		break;
		default:
			if (b_pos + 1 >= sizeof(buf)) {
				tst_res(TINFO, "WARNING: Buffer overflowed while parsing /proc/cmdline");
				while (line[l_pos] != '\0' && line[l_pos] != ' ' && line[l_pos] != '\n')
					l_pos++;

				var_id = -1;
				b_pos = 0;

				if (line[l_pos] != '\0')
					l_pos--;
			} else {
				buf[b_pos++] = c;
			}
		break;
		}
	}

	for (i = 0; i < params_len; i++) {
		if (params[i].found)
			tst_res(TINFO, "%s is found in /proc/cmdline", params[i].key);
		else
			tst_res(TINFO, "%s is not found in /proc/cmdline", params[i].key);
	}

	SAFE_FCLOSE(f);
}
