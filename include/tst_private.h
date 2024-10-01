// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2019 Petr Vorel <pvorel@suse.cz>
 *
 * Internal helper functions for the shell library. Do not use directly
 * in test programs.
 */

#ifndef TST_PRIVATE_H_
#define TST_PRIVATE_H_

#include <stdio.h>
#include <netdb.h>
#include "tst_defaults.h"

#define MAX_IPV4_PREFIX 32
#define MAX_IPV6_PREFIX 128

#define tst_res_comment(...) { \
	fprintf(stderr, "# "); \
	tst_res(__VA_ARGS__); } \


#define tst_brk_comment(...) { \
	fprintf(stderr, "# "); \
	tst_brk(TCONF, __VA_ARGS__); } \

void tst_print_svar(const char *name, const char *val);
void tst_print_svar_change(const char *name, const char *val);

int tst_get_prefix(const char *ip_str, int is_ipv6);

/*
 * Checks kernel config for a single configuration option and returns its
 * state if found. The possible return values are the same as for
 * tst_kconfig_var.choice, with the same meaning. See tst_kconfig_read()
 * description in tst_kconfig.h.
 */
char tst_kconfig_get(const char *confname);

/*
 * If cmd argument is a single command, this function just checks command
 * whether exists. If not, case breaks if brk_nosupp is defined.
 * If cmd argument is a complex string ie 'mkfs.ext4 >= 1.43.0', this
 * function checks command version whether meets this requirement.
 * If not, case breaks if brk_nosupp is defined.
 */
int tst_check_cmd(const char *cmd, const int brk_nosupp);

#endif
