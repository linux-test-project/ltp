/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
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
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define WRITE_STRING(fd, string) if (write(fd, string, sizeof(string)-1)) {}

int main(int argc, char *argv[])
{
	int i, pos = 0;
	char *args[argc];
	char *stdin_path = NULL, *stdout_path = NULL, *stderr_path = NULL;
	int stderr_fd = 0;

	if (argc <= 1) {
		fprintf(stderr, "%s: Takes at least one agrument!\n", argv[0]);
		return 1;
	}

	for (i = 1; i < argc; i++) {

		if (argv[i][0] == '>') {
			if (argv[i][1]) {
				stdout_path = argv[i]+1;
			} else {
				if (++i >= argc) {
					fprintf(stderr,
					        "%s: Missing filename after >\n",
					        argv[0]);
					return 1;
				}
				stdout_path = argv[i];
			}
			continue;
		}

		if (argv[i][0] == '<') {
			if (argv[i][1]) {
				stdin_path = argv[i]+1;
			} else {
				if (++i >= argc) {
					fprintf(stderr,
					        "%s: Missing filename after <\n",
					        argv[0]);
					return 1;
				}
				stdin_path = argv[i];
			}
			continue;
		}

		if (argv[i][0] == '2' && argv[i][1] == '>') {
			if (argv[i][2]) {
				stderr_path = argv[i]+2;
			} else {
				if (++i >= argc) {
					fprintf(stderr,
					        "%s: Missing filename after 2>\n",
					        argv[0]);
					return 1;
				}
				stderr_path = argv[i];
			}
			continue;
		}

		args[pos++] = argv[i];
	}

	args[pos] = NULL;

	if (stdin_path) {
		if (close(0)) {
			fprintf(stderr, "%s: Failed to close stdin: %s\n",
				argv[0], strerror(errno));
			return 1;
		}
		if (open(stdin_path, O_RDONLY) < 0) {
			fprintf(stderr,
			        "%s: Failed to open '%s' for reading: %s\n",
				argv[0], stdin_path, strerror(errno));
			return 1;
		}
	}

	if (stdout_path) {
		if (close(1)) {
			fprintf(stderr, "%s: Failed to close stdout: %s\n",
				argv[0], strerror(errno));
			return 1;
		}
		if (open(stdout_path, O_CREAT|O_WRONLY|O_TRUNC, 0777) < 0) {
			fprintf(stderr,
			        "%s: Failed to open '%s' for writing: %s\n",
				argv[0], stdin_path, strerror(errno));
			return 1;
		}
	}

	if (stderr_path) {
		int fd = open(stderr_path, O_CREAT|O_WRONLY|O_TRUNC, 0777);
		int stderr_fd = dup(2);

		if (stderr_fd < 0) {
			fprintf(stderr, "%s: Failed to dup() stderr: %s\n",
				argv[0], strerror(errno));
			return 1;
		}

		if (fd < 0) {
			fprintf(stderr,
			        "%s: Failed to open '%s' for writing: %s\n",
				argv[0], stdin_path, strerror(errno));
			return 1;
		}

		if (dup2(fd, 2) < 0) {
			fprintf(stderr, "%s: Failed to dup2 stderr: %s\n",
				argv[0], strerror(errno));
			WRITE_STRING(stderr_fd, "Failed to dup2 stderr\n");
			return 1;
		}
	}

	execvp(argv[1], args);

	/* Fall back to shell if command wasn't found */
	FILE *sin = popen("/bin/sh", "w");

	if (!sin) {
		if (stderr_fd) {
			WRITE_STRING(stderr_fd, "Failed to popen /bin/sh\n");
		} else {
			fprintf(stderr, "%s: Failed to popen /bin/sh: %s\n",
				argv[0], strerror(errno));
		}
		return 1;
	}

	//TODO: Should we escape args?
	for (i = 0; args[i]; i++)
		fprintf(sin, "%s ", args[i]);

	return pclose(sin);
}
