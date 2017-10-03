/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: t -*- */
/*
 * self_exec.c: self_exec magic required to run child functions on uClinux
 *
 * Copyright (C) 2005 Paul J.Y. Lahaie <pjlahaie-at-steamballoon.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * This software was produced by Steamballoon Incorporated
 * 55 Byward Market Square, 2nd Floor North, Ottawa, ON K1N 9C3, Canada
 */

#define _GNU_SOURCE		/* for asprintf */

#include "config.h"

#ifdef UCLINUX

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "test.h"
#include "safe_macros.h"

/* Set from parse_opts.c: */
char *child_args;		/* Arguments to child when -C is used */

static char *start_cwd;		/* Stores the starting directory for self_exec */

int asprintf(char **app, const char *fmt, ...)
{
	va_list ptr;
	int rv;
	char *p;

	/*
	 * First iteration - find out size of buffer required and allocate it.
	 */
	va_start(ptr, fmt);
	rv = vsnprintf(NULL, 0, fmt, ptr);
	va_end(ptr);

	p = malloc(++rv);	/* allocate the buffer */
	*app = p;
	if (!p) {
		return -1;
	}

	/*
	 * Second iteration - actually produce output.
	 */
	va_start(ptr, fmt);
	rv = vsnprintf(p, rv, fmt, ptr);
	va_end(ptr);

	return rv;
}

void maybe_run_child(void (*child) (), const char *fmt, ...)
{
	va_list ap;
	char *child_dir;
	char *p, *tok;
	int *iptr, i, j;
	char *s;
	char **sptr;
	char *endptr;

	/* Store the current directory for later use. */
	start_cwd = getcwd(NULL, 0);

	if (child_args) {
		char *args = strdup(child_args);

		child_dir = strtok(args, ",");
		if (strlen(child_dir) == 0) {
			tst_brkm(TBROK, NULL,
				 "Could not get directory from -C option");
			return;
		}

		va_start(ap, fmt);

		for (p = fmt; *p; p++) {
			tok = strtok(NULL, ",");
			if (!tok || strlen(tok) == 0) {
				tst_brkm(TBROK, NULL,
					 "Invalid argument to -C option");
				return;
			}

			switch (*p) {
			case 'd':
				iptr = va_arg(ap, int *);
				i = strtol(tok, &endptr, 10);
				if (*endptr != '\0') {
					tst_brkm(TBROK, NULL,
						 "Invalid argument to -C option");
					return;
				}
				*iptr = i;
				break;
			case 'n':
				j = va_arg(ap, int);
				i = strtol(tok, &endptr, 10);
				if (*endptr != '\0') {
					tst_brkm(TBROK, NULL,
						 "Invalid argument to -C option");
					return;
				}
				if (j != i) {
					va_end(ap);
					free(args);
					return;
				}
				break;
			case 's':
				s = va_arg(ap, char *);
				if (!strncpy(s, tok, strlen(tok) + 1)) {
					tst_brkm(TBROK, NULL,
						 "Could not strncpy for -C option");
					return;
				}
				break;
			case 'S':
				sptr = va_arg(ap, char **);
				*sptr = strdup(tok);
				if (!*sptr) {
					tst_brkm(TBROK, NULL,
						 "Could not strdup for -C option");
					return;
				}
				break;
			default:
				tst_brkm(TBROK, NULL,
					 "Format string option %c not implemented",
					 *p);
				return;
			}
		}

		va_end(ap);
		free(args);
		SAFE_CHDIR(NULL, child_dir);

		(*child) ();
		tst_resm(TWARN, "Child function returned unexpectedly");
		/* Exit here? or exit silently? */
	}
}

int self_exec(const char *argv0, const char *fmt, ...)
{
	va_list ap;
	char *p;
	char *tmp_cwd;
	char *arg;
	int ival;
	char *str;

	if ((tmp_cwd = getcwd(NULL, 0)) == NULL) {
		tst_resm(TBROK, "Could not getcwd()");
		return -1;
	}

	arg = strdup(tmp_cwd);
	if (arg == NULL) {
		tst_resm(TBROK, "Could not produce self_exec string");
		return -1;
	}

	va_start(ap, fmt);

	for (p = fmt; *p; p++) {
		switch (*p) {
		case 'd':
		case 'n':
			ival = va_arg(ap, int);
			if (asprintf(&arg, "%s,%d", arg, ival) < 0) {
				tst_resm(TBROK,
					 "Could not produce self_exec string");
				return -1;
			}
			break;
		case 's':
		case 'S':
			str = va_arg(ap, char *);
			if (asprintf(&arg, "%s,%s", arg, str) < 0) {
				tst_resm(TBROK,
					 "Could not produce self_exec string");
				return -1;
			}
			break;
		default:
			tst_resm(TBROK,
				 "Format string option %c not implemented", *p);
			return -1;
			break;
		}
	}

	va_end(ap);

	if (chdir(start_cwd) < 0) {
		tst_resm(TBROK, "Could not change to %s for self_exec",
			 start_cwd);
		return -1;
	}

	return execlp(argv0, argv0, "-C", arg, (char *)NULL);
}

#endif /* UCLINUX */
