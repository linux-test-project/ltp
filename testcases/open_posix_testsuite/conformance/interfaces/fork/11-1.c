/*
 * Copyright © 2017 SUSE LLC
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; If not, see <http://www.gnu.org/licenses/>.
 *
 * Tests that file locks are not inherited by the child process after a fork.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#include "posixtest.h"

static int child(int fd)
{
	struct flock fl = {
		.l_type = F_WRLCK,
		.l_whence = SEEK_SET,
		.l_start = 1,
		.l_len = 99
	};

	if (fcntl(fd, F_GETLK, &fl) == -1) {
		printf("Could not get lock: %s (%d)\n",
		       strerror(errno), errno);
		return PTS_UNRESOLVED;
	}

	if (fl.l_type == F_UNLCK) {
		printf("Child found lock is F_UNLCK, should be F_WRLCK\n");
		return PTS_FAIL;
	}

	if (fcntl(fd, F_SETLK, &fl) == -1) {
		if (errno == EACCES || errno == EAGAIN) {
			printf("PASSED: child did not inherit the lock\n");
			return PTS_PASS;
		}

		printf("Unexpected fcntl error: %s (%d)\n",
		       strerror(errno), errno);
		return PTS_UNRESOLVED;
	}

	printf("Child locked file already locked by parent\n");
	return PTS_FAIL;
}

int main(void)
{
	char path_template[] = "/tmp/fork-11-1-XXXXXX";
	int fd, child_stat, result = PTS_UNRESOLVED;
	pid_t child_pid;
	struct flock fl = {
		.l_type = F_WRLCK,
		.l_whence = SEEK_SET,
		.l_start = 0,
		.l_len = 100,
	};

	fd = mkstemp(path_template);
	if (fd == -1) {
		printf("Could not open temporary file: %s (%d)\n",
		       strerror(errno), errno);
		return result;
	}

	if (fcntl(fd, F_SETLK, &fl) == -1) {
		printf("Could not set initial lock: %s (%d)\n",
		       strerror(errno), errno);
		goto cleanup;
	}

	child_pid = fork();
	if (child_pid == -1) {
		printf("Fork failed: %s (%d)\n", strerror(errno), errno);
		goto cleanup;
	}

	if (child_pid == 0)
		exit(child(fd));

	if (waitpid(child_pid, &child_stat, 0) == -1) {
		printf("Wait failed: %s (%d)\n", strerror(errno), errno);
		goto cleanup;
	}

	if (WIFEXITED(child_stat))
		result = WEXITSTATUS(child_stat);
	else
		printf("Child terminated abnormally!\n");

cleanup:
	close(fd);
	return result;
}
