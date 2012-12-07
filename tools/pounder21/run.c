/*
 * The main pounder process controller and scheduler program.
 * Author: Darrick Wong <djwong@us.ibm.com>
 */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

#include "proclist.h"
#include "debug.h"

// List of subprocesses to wait upon
struct proclist_t wait_ons = { NULL };
struct proclist_t daemons = { NULL };

static int is_leader = 0;
static char *pidfile = "";

static inline int is_executable(const char *fname);
static inline int is_directory(const char *fname);
static inline int test_filter(const struct dirent *p);
static inline int test_sort(const struct dirent **a, const struct dirent **b);
static int wait_for_pids(void);
static void wait_for_daemons(void);
static void note_process(pid_t pid, char *name);
static void note_daemon(pid_t pid, char *name);
static void kill_tests(void);
static void kill_daemons(void);
static int process_dir(const char *fname);
static pid_t spawn_test(char *fname);
static void note_child(pid_t pid, char *fname, char type);
static int child_finished(const char *name, int stat);
static char *progname;

#define TEST_PATH_LEN 512
#define TEST_FORK_WAIT 100

/**
 * Kill everything upon ^C.
 */
static void jump_out(int signum)
{
	pounder_fprintf(stdout, "Control-C received; aborting!\n");
	//unlink("pounder_pgrp");
	kill_tests();
	kill_daemons();
	if (is_leader) {
		unlink(pidfile);
	}
	exit(0);
}

/**
 * Kills tests launched from within.
 */
static void kill_tests(void)
{
	struct proclist_item_t *curr;

	curr = wait_ons.head;
	while (curr != NULL) {
		kill(-curr->pid, SIGTERM);
		curr = curr->next;
	}
}

/**
 * Kills daemons launched from within.
 */
static void kill_daemons(void)
{
	struct proclist_item_t *curr;

	curr = daemons.head;
	while (curr != NULL) {
		kill(-curr->pid, SIGTERM);
		curr = curr->next;
	}
}

/**
 * Record the pounder leader's PID in a file.
 */
static void record_pid(void)
{
	FILE *fp;

	pidfile = getenv("POUNDER_PIDFILE");
	if (pidfile == NULL) {
		pidfile = "pounder.pid";
	}

	fp = fopen(pidfile, "w");
	if (fp == NULL) {
		perror(pidfile);
	}
	fprintf(fp, "%d", getpid());
	fclose(fp);
}

/**
 * Main program.  Returns 1 if all programs run successfully, 0 if
 * something failed and -1 if there was an error running programs.
 */
int main(int argc, char *argv[])
{
	int retcode;
	struct sigaction zig;
	pid_t pid;
	char *c;

	/* Check parameters */
	if (argc < 2) {
		fprintf(stderr, "Usage: %s test_prog\n", argv[0]);
		return 1;
	}

	if (argc > 2 && strcmp(argv[2], "--leader") == 0) {
		pounder_fprintf(stdout,
				"Logging this test output to %s/POUNDERLOG.\n",
				getenv("POUNDER_LOGDIR"));
		is_leader = 1;
		record_pid();
	}

	progname = argv[0];

	/* Set up signals */
	memset(&zig, 0x00, sizeof(zig));
	zig.sa_handler = jump_out;
	sigaction(SIGHUP, &zig, NULL);
	sigaction(SIGINT, &zig, NULL);
	sigaction(SIGTERM, &zig, NULL);

	if (is_directory(argv[1])) {
		retcode = process_dir(argv[1]);
	} else {
		if (is_executable(argv[1])) {
			c = rindex(argv[1], '/');
			c++;

			// Start the test
			pid = spawn_test(argv[1]);
			if (pid < 0) {
				perror("fork");
				retcode = -1;
				goto out;
			}
			// Track the test
			note_process(pid, argv[1]);
			if (wait_for_pids() == 0) {
				retcode = 1;
			} else {
				retcode = 0;
			}
		} else {
			pounder_fprintf(stderr,
					"%s: Not a directory or a test.\n",
					argv[1]);
			retcode = -1;
		}
	}

out:
	kill_daemons();
	wait_for_daemons();
	if (is_leader) {
		if (retcode == 0) {
			pounder_fprintf(stdout, "%s: %s.\n", argv[1], pass_msg);
		} else if (retcode < 0 || retcode == 255) {
			pounder_fprintf(stdout, "%s: %s with code %d.\n",
					argv[1], abort_msg, retcode);
		} else {
			pounder_fprintf(stdout, "%s: %s with code %d.\n",
					argv[1], fail_msg, retcode);
		}
		unlink(pidfile);
	}
	exit(retcode);
}

/**
 * Helper function to determine if a file is executable.
 * Returns 1 if yes, 0 if no and -1 if error.
 */
static inline int is_executable(const char *fname)
{
	struct stat tmp;

	if (stat(fname, &tmp) < 0) {
		return -1;
	}

	if (geteuid() == 0) {
		return 1;
	} else if (geteuid() == tmp.st_uid) {
		return tmp.st_mode & S_IXUSR;
	} else if (getegid() == tmp.st_gid) {
		return tmp.st_mode & S_IXGRP;
	} else {
		return tmp.st_mode & S_IXOTH;
	}
}

/**
 * Helper function to determine if a file is a directory.
 * Returns 1 if yes, 0 if no and -1 if error.
 */
static inline int is_directory(const char *fname)
{
	struct stat tmp;

	if (stat(fname, &tmp) < 0) {
		return 0;
	}

	return S_ISDIR(tmp.st_mode);
}

/**
 * Returns 1 if the directory entry's filename fits the test name pattern.
 */
static inline int test_filter(const struct dirent *p)
{
	return ((p->d_name[0] == 'T' || p->d_name[0] == 'D')
		&& isdigit(p->d_name[1]) && isdigit(p->d_name[2]));
}

/**
 * Simple routine to compare two tests names such that lower number/name pairs
 * are considered "lesser" values.
 */
//static inline int test_sort(const struct dirent **a, const struct dirent **b) {
static inline int test_sort(const struct dirent **a, const struct dirent **b)
{
	return strcmp(&(*b)->d_name[1], &(*a)->d_name[1]);
}

/**
 * Takes the wait() status integer and prints a log message.
 * Returns 1 if there was a failure.
 */
static int child_finished(const char *name, int stat)
{
	int x;
	// did we sig-exit?
	if (WIFSIGNALED(stat)) {
		pounder_fprintf(stdout, "%s: %s on signal %d.\n",
				name, fail_msg, WTERMSIG(stat));
		return 1;
	} else {
		x = WEXITSTATUS(stat);
		if (x == 0) {
			pounder_fprintf(stdout, "%s: %s.\n", name, pass_msg);
			return 0;
		} else if (x < 0 || x == 255) {
			pounder_fprintf(stdout, "%s: %s with code %d.\n",
					name, abort_msg, x);
			return 1;
			// FIXME: add test to blacklist
		} else {
			pounder_fprintf(stdout, "%s: %s with code %d.\n",
					name, fail_msg, x);
			return 1;
		}
	}
}

/**
 * Wait for some number of PIDs.  If any of them return nonzero, we
 * assume that there was some kind of failure and return 0.  Otherwise,
 * we return 1 to indicate success.
 */
static int wait_for_pids(void)
{
	struct proclist_item_t *curr;
	int i, stat, res, nprocs;
	pid_t pid;

	res = 1;

	// figure out how many times we have to wait...
	curr = wait_ons.head;
	nprocs = 0;
	while (curr != NULL) {
		nprocs++;
		curr = curr->next;
	}

	// now wait for children.
	for (i = 0; i < nprocs;) {
		pid = wait(&stat);

		if (pid < 0) {
			perror("wait");
			return 0;
		}
		// go find the child
		curr = wait_ons.head;
		while (curr != NULL) {
			if (curr->pid == pid) {
				res =
				    (child_finished(curr->name, stat) ? 0 :
				     res);

				// one less pid to wait for
				i++;

				// stop observing
				remove_from_proclist(&wait_ons, curr);
				free(curr->name);
				free(curr);
				break;
			}
			curr = curr->next;
		}

		curr = daemons.head;
		while (curr != NULL) {
			if (curr->pid == pid) {
				child_finished(curr->name, stat);
				remove_from_proclist(&daemons, curr);
				free(curr->name);
				free(curr);
				break;
			}
			curr = curr->next;
		}
	}

	return res;
}

/**
 * Wait for daemons to finish.  This function does NOT wait for wait_ons.
 */
static void wait_for_daemons(void)
{
	struct proclist_item_t *curr;
	int i, stat, res, nprocs;
	pid_t pid;

	res = 1;

	// figure out how many times we have to wait...
	curr = daemons.head;
	nprocs = 0;
	while (curr != NULL) {
		nprocs++;
		curr = curr->next;
	}

	// now wait for daemons.
	for (i = 0; i < nprocs;) {
		pid = wait(&stat);

		if (pid < 0) {
			perror("wait");
			if (errno == ECHILD) {
				return;
			}
		}

		curr = daemons.head;
		while (curr != NULL) {
			if (curr->pid == pid) {
				child_finished(curr->name, stat);
				i++;
				remove_from_proclist(&daemons, curr);
				free(curr->name);
				free(curr);
				break;
			}
			curr = curr->next;
		}
	}
}

/**
 * Creates a record of processes that we want to watch for.
 */
static void note_process(pid_t pid, char *name)
{
	struct proclist_item_t *it;

	it = calloc(1, sizeof(struct proclist_item_t));
	if (it == NULL) {
		perror("malloc proclist_item_t");
		// XXX: Maybe we should just waitpid?
		return;
	}
	it->pid = pid;
	it->name = calloc(strlen(name) + 1, sizeof(char));
	if (it->name == NULL) {
		perror("malloc procitem name");
		// XXX: Maybe we should just waitpid?
		return;
	}
	strcpy(it->name, name);

	add_to_proclist(&wait_ons, it);
}

/**
 * Creates a record of daemons that should be killed on exit.
 */
static void note_daemon(pid_t pid, char *name)
{
	struct proclist_item_t *it;

	it = calloc(1, sizeof(struct proclist_item_t));
	if (it == NULL) {
		perror("malloc proclist_item_t");
		// XXX: what do we do here?
		return;
	}
	it->pid = pid;
	it->name = calloc(strlen(name) + 1, sizeof(char));
	if (it->name == NULL) {
		perror("malloc procitem name");
		// XXX: what do we do here?
		return;
	}
	strcpy(it->name, name);

	add_to_proclist(&daemons, it);
}

/**
 * Starts a test, with the stdin/out/err fd's redirected to logs.
 * The 'fname' parameter should be a relative path from $POUNDER_HOME.
 */
static pid_t spawn_test(char *fname)
{
	pid_t pid;
	int fd, tmp;
	char buf[TEST_PATH_LEN], buf2[TEST_PATH_LEN];
	char *last_slash;

	pid = fork();
	if (pid == 0) {
		if (setpgrp() < 0) {
			perror("setpgid");
		}

		pounder_fprintf(stdout, "%s: %s test.\n", fname, start_msg);

		// reroute stdin
		fd = open("/dev/null", O_RDWR);
		if (fd < 0) {
			perror("/dev/null");
			exit(-1);
		}
		close(0);
		tmp = dup2(fd, 0);
		if (tmp < 0) {
			perror("dup(/dev/null)");
			exit(-1);
		}
		close(fd);

		// generate log name-- '/' -> '-'.
		snprintf(buf2, TEST_PATH_LEN, "%s|%s",
			 getenv("POUNDER_LOGDIR"), fname);

		fd = strlen(buf2);
		for (tmp = (index(buf2, '|') - buf2); tmp < fd; tmp++) {
			if (buf2[tmp] == '/') {
				buf2[tmp] = '-';
			} else if (buf2[tmp] == '|') {
				buf2[tmp] = '/';
			}
		}

		// make it so that we have a way to get back to the
		// original console.
		tmp = dup2(1, 3);
		if (tmp < 0) {
			perror("dup(stdout, 3)");
			exit(-1);
		}
		// reroute stdout/stderr
		fd = open(buf2, O_RDWR | O_CREAT | O_TRUNC | O_SYNC,
			  S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
		if (fd < 0) {
			perror(buf2);
			exit(-1);
		}
		close(1);
		tmp = dup2(fd, 1);
		if (tmp < 0) {
			perror("dup(log, 1)");
			exit(-1);
		}
		close(2);
		tmp = dup2(fd, 2);
		if (tmp < 0) {
			perror("dup(log, 2)");
			exit(-1);
		}
		close(fd);

		// let us construct the absolute pathname of the test.
		// first find the current directory
		if (getcwd(buf, TEST_PATH_LEN) == NULL) {
			perror("getcwd");
			exit(-1);
		}
		// then splice cwd + fname
		snprintf(buf2, TEST_PATH_LEN, "%s/%s", buf, fname);

		// find the location of the last slash
		last_slash = rindex(buf2, '/');

		if (last_slash != NULL) {
			// copy the filename part into a new buffer
			snprintf(buf, TEST_PATH_LEN, "./%s", last_slash + 1);

			// truncate at the last slash
			*last_slash = 0;

			// and chdir
			if (chdir(buf2) != 0) {
				perror(buf2);
				exit(-1);
			}
			// reassign variables
			fname = buf;
		}
		// spawn the process
		execlp(fname, fname, NULL);

		// If we get here, we can't run the test.
		perror(fname);
		exit(-1);
	}

	tmp = errno;
	/* yield for a short while, so that the test has
	 * a little bit of time to run.
	 */
	usleep(TEST_FORK_WAIT);
	errno = tmp;

	return pid;
}

/**
 * Adds a child process to either the running-test or running-daemon
 * list.
 */
static void note_child(pid_t pid, char *fname, char type)
{
	if (type == 'T') {
		note_process(pid, fname);
	} else if (type == 'D') {
		note_daemon(pid, fname);
	} else {
		pounder_fprintf(stdout,
				"Don't know what to do with child `%s' of type %c.\n",
				fname, type);
	}
}

/**
 * Process a directory--for each entry in a directory, execute files or spawn
 * a new copy of ourself on the new directory.  Process execution is subject to
 * these rules:
 *
 * - Test files that start with the same number '00foo' and '00bar' are allowed
 *   to run simultaneously.
 * - Test files are run in order of number and then name.
 *
 * If a the fork fails, bit 1 of the return code is set.  If a
 * program runs but fails, bit 2 is set.
 */
static int process_dir(const char *fname)
{
	struct dirent **namelist;
	int i, result = 0;
	char buf[TEST_PATH_LEN];
	int curr_level_num = -1;
	int test_level_num;
	pid_t pid;
	int children_ok = 1;

	pounder_fprintf(stdout, "%s: Entering directory.\n", fname);

	i = scandir(fname, &namelist, test_filter,
		    (int (*)(const void *, const void *))test_sort);
	if (i < 0) {
		perror(fname);
		return -1;
	}

	while (i--) {
		/* determine level number */
		test_level_num = ((namelist[i]->d_name[1] - '0') * 10)
		    + (namelist[i]->d_name[2] - '0');

		if (curr_level_num == -1) {
			curr_level_num = test_level_num;
		}

		if (curr_level_num != test_level_num) {
			children_ok &= wait_for_pids();
			curr_level_num = test_level_num;
		}

		snprintf(buf, TEST_PATH_LEN, "%s/%s", fname,
			 namelist[i]->d_name);
		if (is_directory(buf)) {
			pid = fork();
			if (pid == 0) {
				if (setpgrp() < 0) {
					perror("setpgid");
				}
				// spawn a new copy of ourself.
				execl(progname, progname, buf, NULL);

				perror(progname);
				exit(-1);
			}
		} else {
			pid = spawn_test(buf);
		}

		if (pid < 0) {
			perror("fork");
			result |= 1;
			free(namelist[i]);
			continue;
		}

		note_child(pid, buf, namelist[i]->d_name[0]);

		free(namelist[i]);
	}
	free(namelist);

	/* wait for remaining runners */
	children_ok &= wait_for_pids();
	if (children_ok == 0) {
		result |= 2;
	}

	pounder_fprintf(stdout, "%s: Leaving directory.\n", fname);

	return result;
}
