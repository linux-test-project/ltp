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
#include <errno.h>
#include "config.h"
#ifdef HAVE_SYS_FANOTIFY_H
# include <sys/fanotify.h>
#endif
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/personality.h"

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

int safe_fanotify_init(const char *file, const int lineno,
	unsigned int flags, unsigned int event_f_flags)
{
	int rval;

#ifdef HAVE_SYS_FANOTIFY_H
	rval = fanotify_init(flags, event_f_flags);

	if (rval == -1) {
		if (errno == ENOSYS) {
			tst_brk(TCONF,
				"fanotify is not configured in this kernel.");
		}
		tst_brk(TBROK | TERRNO,
			"%s:%d: fanotify_init() failed", file, lineno);
	}
#else
	tst_brk(TCONF, "Header <sys/fanotify.h> is not present");
#endif /* HAVE_SYS_FANOTIFY_H */

	return rval;
}

int safe_personality(const char *filename, unsigned int lineno,
		    unsigned long persona)
{
	int prev_persona = personality(persona);

	if (prev_persona < 0) {
		tst_brk_(filename, lineno, TBROK | TERRNO,
			 "persona(%ld) failed", persona);
	}

	return prev_persona;
}
