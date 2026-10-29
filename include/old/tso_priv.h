// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Cyril Hrubis chrubis@suse.cz
 * Copyright (c) Linux Test Project, 2026
 */

#ifndef __LTP_PRIV_H__
#define __LTP_PRIV_H__

#include <stdarg.h>
#include "tst_defaults.h"

/* environment variables for controlling  tst_res verbosity */
#define TOUT_VERBOSE_S  "VERBOSE"	/* All test cases reported */
#define TOUT_NOPASS_S   "NOPASS"	/* No pass test cases are reported */
#define TOUT_DISCARD_S  "DISCARD"	/* No output is reported */

#define USC_ITERATION_ENV       "USC_ITERATIONS"
#define USC_LOOP_WALLTIME	"USC_LOOP_WALLTIME"
#define USC_NO_FUNC_CHECK	"USC_NO_FUNC_CHECK"
#define USC_LOOP_DELAY		"USC_LOOP_DELAY"

const char *parse_opts(int ac, char **av, const option_t *user_optarr, void
                       (*uhf)(void));

/* Interface for rerouting to new lib calls from tst_res.c */
extern void *tst_test;

void tst_vbrk_(const char *file, const int lineno, int ttype,
               const char *fmt, va_list va) __attribute__((noreturn));

void tst_brk_(const char *file, const int lineno, int ttype,
              const char *msg, ...);

void tst_vres_(const char *file, const int lineno, int ttype,
               const char *fmt, va_list va);

void tst_res_(const char *file, const int lineno, int ttype,
              const char *msg, ...);


#define NO_NEWLIB_ASSERT(file, lineno)                                \
	if (tst_test) {                                               \
		tst_brk_(file, lineno, TBROK,                         \
			 "%s() executed from newlib!", __FUNCTION__); \
	}

#endif /* __LTP_PRIV_H__ */
