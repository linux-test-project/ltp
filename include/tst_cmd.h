/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_CMD_H__
#define TST_CMD_H__

enum tst_cmd_flags {
	/*
	 * return the program exit code, otherwise it will call cleanup_fn() if the
	 * program exit code is not zero.
	 */
	TST_CMD_PASS_RETVAL = 1,

	/* exit with TCONF if program is not in path */
	TST_CMD_TCONF_ON_MISSING = 2,
};

/*
 * vfork() + execvp() specified program.
 * @argv: a list of two (at least program name + NULL) or more pointers that
 * represent the argument list to the new program. The array of pointers
 * must be terminated by a NULL pointer.
 * @stdout_fd: file descriptor where to redirect stdout. Set -1 if
 * redirection is not needed.
 * @stderr_fd: file descriptor where to redirect stderr. Set -1 if
 * redirection is not needed.
 * @flags: enum tst_cmd_flags
 */
int tst_cmd_fds_(void (cleanup_fn)(void),
			const char *const argv[],
			int stdout_fd,
			int stderr_fd,
			enum tst_cmd_flags flags);

/* Executes tst_cmd_fds() and redirects its output to a file
 * @stdout_path: path where to redirect stdout. Set NULL if redirection is
 * not needed.
 * @stderr_path: path where to redirect stderr. Set NULL if redirection is
 * not needed.
 * @flags: enum tst_cmd_flags
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

/* Wrapper function for system(3), ignorcing SIGCHLD signal.
 * @command: the command to be run.
 */
int tst_system(const char *command);

#endif	/* TST_CMD_H__ */
