/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_CMD_H__
#define TST_CMD_H__

/*
 * vfork() + execvp() specified program.
 * @argv: a list of two (at least program name + NULL) or more pointers that
 * represent the argument list to the new program. The array of pointers
 * must be terminated by a NULL pointer.
 * @stdout_fd: file descriptor where to redirect stdout. Set -1 if
 * redirection is not needed.
 * @stderr_fd: file descriptor where to redirect stderr. Set -1 if
 * redirection is not needed.
 * @pass_exit_val: if it's non-zero, this function will return the program
 * exit code, otherwise it will call cleanup_fn() if the program
 * exit code is not zero.
 */
int tst_run_cmd_fds_(void (cleanup_fn)(void),
			const char *const argv[],
			int stdout_fd,
			int stderr_fd,
			int pass_exit_val);

/* Executes tst_run_cmd_fds() and redirects its output to a file
 * @stdout_path: path where to redirect stdout. Set NULL if redirection is
 * not needed.
 * @stderr_path: path where to redirect stderr. Set NULL if redirection is
 * not needed.
 * @pass_exit_val: if it's non-zero, this function will return the program
 * exit code, otherwise it will call cleanup_fn() if the program
 * exit code is not zero.
 */
int tst_run_cmd_(void (cleanup_fn)(void),
		const char *const argv[],
		const char *stdout_path,
		const char *stderr_path,
		int pass_exit_val);

#ifdef TST_TEST_H__
static inline int tst_run_cmd_fds(const char *const argv[],
				  int stdout_fd,
				  int stderr_fd,
				  int pass_exit_val)
{
	return tst_run_cmd_fds_(NULL, argv,
	                        stdout_fd, stderr_fd, pass_exit_val);
}

static inline int tst_run_cmd(const char *const argv[],
			      const char *stdout_path,
			      const char *stderr_path,
			      int pass_exit_val)
{
	return tst_run_cmd_(NULL, argv,
	                    stdout_path, stderr_path, pass_exit_val);
}
#else
static inline int tst_run_cmd_fds(void (cleanup_fn)(void),
				  const char *const argv[],
				  int stdout_fd,
				  int stderr_fd,
				  int pass_exit_val)
{
	return tst_run_cmd_fds_(cleanup_fn, argv,
	                        stdout_fd, stderr_fd, pass_exit_val);
}

static inline int tst_run_cmd(void (cleanup_fn)(void),
			      const char *const argv[],
			      const char *stdout_path,
			      const char *stderr_path,
			      int pass_exit_val)
{
	return tst_run_cmd_(cleanup_fn, argv,
	                    stdout_path, stderr_path, pass_exit_val);
}
#endif

/* Wrapper function for system(3), ignorcing SIGCHLD signal.
 * @command: the command to be run.
 */
int tst_system(const char *command);

#endif	/* TST_CMD_H__ */
