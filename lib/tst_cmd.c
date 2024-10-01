/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
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
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "test.h"
#include "tst_cmd.h"
#include "tst_private.h"

#define OPEN_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define OPEN_FLAGS	(O_WRONLY | O_APPEND | O_CREAT)

int tst_cmd_fds_(void (cleanup_fn)(void),
		const char *const argv[],
		int stdout_fd,
		int stderr_fd,
		enum tst_cmd_flags flags)
{
	int rc;

	if (argv == NULL || argv[0] == NULL) {
		tst_brkm(TBROK, cleanup_fn,
			"argument list is empty at %s:%d", __FILE__, __LINE__);
		return -1;
	}

	/*
	 * The tst_sig() install poisoned signal handlers for all signals the
	 * test is not expected to get.
	 *
	 * So we temporarily disable the handler for sigchild we get after our
	 * child exits so that we don't have to disable it in each test that
	 * uses this interface.
	 */
	void *old_handler = signal(SIGCHLD, SIG_DFL);

	char path[PATH_MAX];

	if (tst_get_path(argv[0], path, sizeof(path))) {
		if (flags & TST_CMD_TCONF_ON_MISSING)
			tst_brkm(TCONF, cleanup_fn, "Couldn't find '%s' in $PATH at %s:%d", argv[0],
				 __FILE__, __LINE__);
		else
			return 255;
	}

	pid_t pid = vfork();
	if (pid == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn, "vfork failed at %s:%d",
			__FILE__, __LINE__);
		return -1;
	}
	if (!pid) {
		/* redirecting stdout and stderr if needed */
		if (stdout_fd != -1) {
			close(STDOUT_FILENO);
			dup2(stdout_fd, STDOUT_FILENO);
		}

		if (stderr_fd != -1) {
			close(STDERR_FILENO);
			dup2(stderr_fd, STDERR_FILENO);
		}

		execvp(argv[0], (char *const *)argv);
		_exit(254);
	}

	int ret = -1;
	if (waitpid(pid, &ret, 0) != pid) {
		tst_brkm(TBROK | TERRNO, cleanup_fn, "waitpid failed at %s:%d",
			__FILE__, __LINE__);
		return -1;
	}

	signal(SIGCHLD, old_handler);

	if (!WIFEXITED(ret)) {
		tst_brkm(TBROK, cleanup_fn, "failed to exec cmd '%s' at %s:%d",
			argv[0], __FILE__, __LINE__);
		return -1;
	}

	rc = WEXITSTATUS(ret);

	if (!(flags & TST_CMD_PASS_RETVAL) && rc) {
		tst_brkm(TBROK, cleanup_fn,
			 "'%s' exited with a non-zero code %d at %s:%d",
			 argv[0], rc, __FILE__, __LINE__);
		return -1;
	}

	return rc;
}

int tst_cmd_(void (cleanup_fn)(void),
		const char *const argv[],
		const char *stdout_path,
		const char *stderr_path,
		enum tst_cmd_flags flags)
{
	int stdout_fd = -1;
	int stderr_fd = -1;
	int rc;

	if (stdout_path != NULL) {
		stdout_fd = open(stdout_path,
				OPEN_FLAGS, OPEN_MODE);

		if (stdout_fd == -1)
			tst_resm(TWARN | TERRNO,
				"open() on %s failed at %s:%d",
				stdout_path, __FILE__, __LINE__);
	}

	if (stderr_path != NULL) {
		stderr_fd = open(stderr_path,
				OPEN_FLAGS, OPEN_MODE);

		if (stderr_fd == -1)
			tst_resm(TWARN | TERRNO,
				"open() on %s failed at %s:%d",
				stderr_path, __FILE__, __LINE__);
	}

	rc = tst_cmd_fds(cleanup_fn, argv, stdout_fd, stderr_fd, flags);

	if ((stdout_fd != -1) && (close(stdout_fd) == -1))
		tst_resm(TWARN | TERRNO,
			"close() on %s failed at %s:%d",
			stdout_path, __FILE__, __LINE__);

	if ((stderr_fd != -1) && (close(stderr_fd) == -1))
		tst_resm(TWARN | TERRNO,
			"close() on %s failed at %s:%d",
			stderr_path, __FILE__, __LINE__);

	return rc;
}

int tst_system(const char *command)
{
	int ret = 0;

	/*
	 *Temporarily disable SIGCHLD of user defined handler, so the
	 *system(3) function will not cause unexpected SIGCHLD signal
	 *callback function for test cases.
	 */
	void *old_handler = signal(SIGCHLD, SIG_DFL);

	ret = system(command);

	signal(SIGCHLD, old_handler);
	return ret;
}

static int mkfs_ext4_version_parser(void)
{
	FILE *f;
	int rc, major, minor, patch;

	f = popen("mkfs.ext4 -V 2>&1", "r");
	if (!f) {
		tst_resm(TWARN, "Could not run mkfs.ext4 -V 2>&1 cmd");
		return -1;
	}

	rc = fscanf(f, "mke2fs %d.%d.%d", &major, &minor, &patch);
	pclose(f);
	if (rc != 3) {
		tst_resm(TWARN, "Unable to parse mkfs.ext4 version");
		return -1;
	}

	return major * 10000 +  minor * 100 + patch;
}

static int mkfs_generic_version_table_get(char *version)
{
	int major, minor, patch;
	int len;

	if (sscanf(version, "%u.%u.%u %n", &major, &minor, &patch, &len) != 3) {
		tst_resm(TWARN, "Illegal version(%s), should use format like 1.43.0", version);
		return -1;
	}

	if (len != (int)strlen(version)) {
		tst_resm(TWARN, "Grabage after version");
		return -1;
	}

	return major * 10000 + minor * 100 + patch;
}

static int mkfs_xfs_version_parser(void)
{
	FILE *f;
	int rc, major, minor, patch;

	f = popen("mkfs.xfs -V 2>&1", "r");
	if (!f) {
		tst_resm(TWARN, "Could not run mkfs.xfs -V 2>&1 cmd");
		return -1;
	}

	rc = fscanf(f, "mkfs.xfs version %d.%d.%d", &major, &minor, &patch);
	pclose(f);
	if (rc != 3) {
		tst_resm(TWARN, "Unable to parse mkfs.xfs version");
		return -1;
	}

	return major * 10000 +  minor * 100 + patch;
}

static struct version_parser {
	const char *cmd;
	int (*parser)(void);
	int (*table_get)(char *version);
} version_parsers[] = {
	{"mkfs.ext4", mkfs_ext4_version_parser, mkfs_generic_version_table_get},
	{"mkfs.xfs", mkfs_xfs_version_parser, mkfs_generic_version_table_get},
	{},
};

int tst_check_cmd(const char *cmd, const int brk_nosupp)
{
	struct version_parser *p;
	char *cmd_token, *op_token, *version_token, *next, *str;
	char path[PATH_MAX];
	char parser_cmd[100];
	int ver_parser, ver_get;

	strcpy(parser_cmd, cmd);

	cmd_token = strtok_r(parser_cmd, " ", &next);
	op_token = strtok_r(NULL, " ", &next);
	version_token = strtok_r(NULL, " ", &next);
	str = strtok_r(NULL, " ", &next);

	if (tst_get_path(cmd_token, path, sizeof(path)))
		tst_brkm(TCONF, NULL, "Couldn't find '%s' in $PATH", cmd_token);

	if (!op_token)
		return 0;

	if (!version_token || str) {
		tst_brkm(TCONF, NULL,
			"Illegal format(%s), should use format like mkfs.ext4 >= 1.43.0",
			cmd);
	}

	for (p = &version_parsers[0]; p->cmd; p++) {
		if (!strcmp(p->cmd, cmd_token)) {
			tst_resm(TINFO, "Parsing %s version", p->cmd);
			break;
		}
	}

	if (!p->cmd) {
		tst_brkm(TBROK, NULL, "No version parser for %s implemented!",
			cmd_token);
	}

	ver_parser = p->parser();
	if (ver_parser < 0)
		tst_brkm(TBROK, NULL, "Failed to parse %s version", p->cmd);

	ver_get = p->table_get(version_token);
	if (ver_get < 0)
		tst_brkm(TBROK, NULL, "Failed to get %s version", p->cmd);

	if (!strcmp(op_token, ">=")) {
		if (ver_parser < ver_get)
			goto error;
	} else if (!strcmp(op_token, ">")) {
		if (ver_parser <= ver_get)
			goto error;
	} else if (!strcmp(op_token, "<=")) {
		if (ver_parser > ver_get)
			goto error;
	} else if (!strcmp(op_token, "<")) {
		if (ver_parser >= ver_get)
			goto error;
	} else if (!strcmp(op_token, "==")) {
		if (ver_parser != ver_get)
			goto error;
	} else if (!strcmp(op_token, "!=")) {
		if (ver_parser == ver_get)
			goto error;
	} else {
		tst_brkm(TCONF, NULL, "Invalid op(%s)", op_token);
	}

	return 0;
error:
	if (brk_nosupp) {
		tst_brkm(TCONF, NULL, "%s requires %s %d, but got %d",
			cmd, op_token, ver_get, ver_parser);
	} else {
		tst_resm(TCONF, "%s requires %s %d, but got %d",
			cmd, op_token, ver_get, ver_parser);
	}

	return 1;
}
