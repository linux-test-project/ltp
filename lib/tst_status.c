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

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

static char buf[32];

const char *exited(int status)
{
	snprintf(buf, sizeof(buf), "exited with %i", WEXITSTATUS(status));

	return buf;
}

const char *signaled(int status)
{
	snprintf(buf, sizeof(buf), "killed by %s", tst_strsig(status));

	return buf;
}

const char *invalid(int status)
{
	snprintf(buf, sizeof(buf), "invalid status 0x%x", status);

	return buf;
}

const char *tst_strstatus(int status)
{
	if (WIFEXITED(status))
		return exited(status);

	if (WIFSIGNALED(status))
		return signaled(status);

	if (WIFSTOPPED(status))
		return "is stopped";

	if (WIFCONTINUED(status))
		return "is resumed";

	return invalid(status);
}
