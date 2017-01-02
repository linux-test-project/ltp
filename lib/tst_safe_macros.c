/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <unistd.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_safe_macros.h"

int safe_setpgid(const char *file, const int lineno, pid_t pid, pid_t pgid)
{
	int rval;

	rval = setpgid(pid, pgid);
	if (rval) {
		tst_brk(TBROK | TERRNO,
		        "%s:%d: setpgid(%i, %i) failed",
			file, lineno, pid, pgid);
	}

	return rval;
}

pid_t safe_getpgid(const char *file, const int lineno, pid_t pid)
{
	pid_t pgid;

	pgid = getpgid(pid);
	if (pgid == -1) {
		tst_brk(TBROK | TERRNO,
			"%s:%d: getpgid(%i) failed", file, lineno, pid);
	}

	return pgid;
}
