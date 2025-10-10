/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2016-2025
 */

#ifndef TST_CMD_H__
#define TST_CMD_H__

/**
 * enum tst_cmd_flags - flags for tst_cmd() and tst_cmd_fds().
 *
 * @TST_CMD_PASS_RETVAL: return the program exit code, otherwise it will call
 * cleanup_fn() if the program exit code is not zero.
 * @TST_CMD_TCONF_ON_MISSING: exit with :c:enum:`TCONF <tst_res_flags>` if
 * program is not in ``PATH``.
 */
enum tst_cmd_flags {
	TST_CMD_PASS_RETVAL = 1,
	TST_CMD_TCONF_ON_MISSING = 2,
};

/*
 * vfork() + execvp() specified program.
 *
 * @param argv A list of two (at least program name + NULL) or more pointers that
 * represent the argument list to the new program. The array of pointers
 * must be terminated by a NULL pointer.
 * @param stdout_fd File descriptor where to redirect stdout. Set -1 if
 * redirection is not needed.
 * @param stderr_fd File descriptor where to redirect stderr. Set -1 if
 * redirection is not needed.
 * @param flags enum tst_cmd_flags.
 * @return The exit status of the program.
 */
int tst_cmd_fds_(void (cleanup_fn)(void),
			const char *const argv[],
			int stdout_fd,
			int stderr_fd,
			enum tst_cmd_flags flags);

/*
 * Executes tst_cmd_fds() and redirects its output to a file.
 *
 * @param argv A list of two (at least program name + NULL) or more pointers that
 * represent the argument list to the new program. The array of pointers
 * must be terminated by a NULL pointer.
 * @param stdout_path Path where to redirect stdout. Set NULL if redirection is
 * not needed.
 * @param stderr_path Path where to redirect stderr. Set NULL if redirection is
 * not needed.
 * @param flags enum tst_cmd_flags.
 * @return The exit status of the program.
 */
int tst_cmd_(void (cleanup_fn)(void),
		const char *const argv[],
		const char *stdout_path,
		const char *stderr_path,
		enum tst_cmd_flags flags);

#ifdef TST_TEST_H__
static inline int tst_cmd_fds(const char *const argv[],
				  int stdout_fd,
				  int stderr_fd,
				  enum tst_cmd_flags flags)
{
	return tst_cmd_fds_(NULL, argv,
	                        stdout_fd, stderr_fd, flags);
}

static inline int tst_cmd(const char *const argv[],
			      const char *stdout_path,
			      const char *stderr_path,
			      enum tst_cmd_flags flags)
{
	return tst_cmd_(NULL, argv,
	                    stdout_path, stderr_path, flags);
}
#else
static inline int tst_cmd_fds(void (cleanup_fn)(void),
				  const char *const argv[],
				  int stdout_fd,
				  int stderr_fd,
				  enum tst_cmd_flags flags)
{
	return tst_cmd_fds_(cleanup_fn, argv,
	                        stdout_fd, stderr_fd, flags);
}

static inline int tst_cmd(void (cleanup_fn)(void),
			      const char *const argv[],
			      const char *stdout_path,
			      const char *stderr_path,
			      enum tst_cmd_flags flags)
{
	return tst_cmd_(cleanup_fn, argv,
	                    stdout_path, stderr_path, flags);
}
#endif

/**
 * tst_system() - Wrapper function for :man3:`system`, ignorcing ``SIGCHLD``
 * signal.
 *
 * @command: The command to be run.
 *
 * Return: The system() return code.
 */
int tst_system(const char *command);

#endif	/* TST_CMD_H__ */
