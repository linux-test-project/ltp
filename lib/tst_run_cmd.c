/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "test.h"

void tst_run_cmd(void (cleanup_fn)(void), char *const argv[])
{
	if (argv == NULL || argv[0] == NULL) {
		tst_brkm(TBROK, cleanup_fn,
			"argument list is empty at %s:%d", __FILE__, __LINE__);
	}

	pid_t pid = vfork();
	if (pid == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn, "vfork failed at %s:%d",
			__FILE__, __LINE__);
	}
	if (!pid)
		_exit(execvp(argv[0], argv));

	int ret = -1;
	if (waitpid(pid, &ret, 0) != pid) {
		tst_brkm(TBROK, cleanup_fn, "waitpid failed at %s:%d",
			__FILE__, __LINE__);
	}

	if (!WIFEXITED(ret) || WEXITSTATUS(ret) != 0) {
		tst_brkm(TBROK, cleanup_fn, "failed to exec cmd '%s' at %s:%d",
			argv[0], __FILE__, __LINE__);
	}
}
