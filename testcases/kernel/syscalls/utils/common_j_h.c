/*
 * Crackerjack Project
 *
 * Copyright (C) 2007-2008, Hitachi, Ltd.
 * Author(s): Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *            Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * $Id: common_j_h.c,v 1.5 2009/11/20 06:48:31 yaberauneya Exp $
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include "include_j_h.h"
#include "test.h"

#define barrier() __asm__ __volatile__("": : :"memory")
#define WITH_SIGNALS_BLOCKED(code) {					\
		sigset_t held_sigs_;					\
		sigfillset(&held_sigs_);				\
		sigprocmask(SIG_SETMASK, &held_sigs_, &held_sigs_);	\
		barrier();						\
		code;							\
		barrier();						\
		sigprocmask(SIG_SETMASK, &held_sigs_, NULL);		\
	}

/*
 * Change user ID
 *
 *   We assume 'test' executable is executed with 'root' permission.
 *   So, if you use this function, you can not return 'root' uid.
 */
int setup_uid(char *uname)
{
	struct passwd *pw;
	int rc;

	pw = getpwnam(uname);
	if (!pw) {
		EPRINTF("getpwnam failed.\n");
		return -1;
	}
	rc = setuid(pw->pw_uid);
	if (rc < 0) {
		EPRINTF("setuid failed.\n");
		return -1;
	}
	return 0;
}

/*
 * Change effective user ID
 */
int setup_euid(char *uname, uid_t * old_uid)
{
	struct passwd *pw;
	int rc;

	*old_uid = geteuid();
	pw = getpwnam(uname);
	if (!pw) {
		EPRINTF("getpwnam failed.\n");
		return -1;
	}
	rc = seteuid(pw->pw_uid);
	if (rc < 0) {
		EPRINTF("seteuid failed.\n");
		return -1;
	}
	return 0;
}

int cleanup_euid(uid_t old_uid)
{
	int rc;

	rc = seteuid(old_uid);
	if (rc < 0) {
		EPRINTF("seteuid failed.\n");
		return -1;
	}
	return 0;
}

static void sigterm_handler(int sig)
{
	_exit(0);
}

/*
 * Generate a child process which will send a signal
 */
pid_t create_sig_proc(unsigned long usec, int sig, unsigned count)
{
	pid_t pid, cpid;

	pid = getpid();
	WITH_SIGNALS_BLOCKED(if ((cpid = fork()) == 0) {
			     tst_sig(NOFORK, SIG_DFL, NULL);
			     signal(SIGTERM, sigterm_handler);}
	) ;
	switch (cpid) {
	case 0:
		while (count-- > 0) {
			usleep(usec);
			if (kill(pid, sig) == -1)
				break;
		}
		_exit(0);
		break;
	case -1:
		EPRINTF("fork failed.\n");
		return cpid;
	default:
		return cpid;
	}
}

/*
 * Create and delete test file
 */
int setup_file(char *testdir, char *fname, char *path)
{
	return _setup_file(testdir, fname, path,
			   O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
}

int _setup_file(char *testdir, char *fname, char *path, int flags, mode_t mode)
{
	int rc;

	sprintf(path, "%s/%s", testdir, fname);
	rc = open(path, flags, mode);
	if (rc < 0) {
		EPRINTF("open failed.\n");
		return -1;
	}
	return rc;
}

int cleanup_file(char *path)
{
	unlink(path);
	return 0;
}

/*
 * Create and delete swap file
 */
/* swap file needs to be more than 40KB */
#define MIN_SWAPFILE_SIZE	(64 * 1024)
int setup_swapfile(char *testdir, char *fname, char *path, size_t size)
{
	int fd = -1, rc;
	size_t r_sz;
	int cmdlen = 256;
	char cmd[cmdlen];
	char *p = NULL;

	sprintf(path, "%s/%s", testdir, fname);
	fd = open(path, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		EPRINTF("open failed.\n");
		goto ERR_EXIT;
	}
	if (size < MIN_SWAPFILE_SIZE) {
		EPRINTF("size too short.\n");
		goto ERR_EXIT;
	}
	p = malloc(size);
	if (!p) {
		EPRINTF("malloc failed.\n");
		goto ERR_EXIT;
	}
	/* Swap file must not have hole area */
	memset(p, 0x5a, size);
	r_sz = (size_t) write(fd, p, size);
	if (r_sz != size) {
		EPRINTF("write failed.\n");
		goto ERR_EXIT;
	}
	snprintf(cmd, cmdlen, "/sbin/mkswap %s > /dev/null 2>&1", path);
	rc = system(cmd);
	if (rc != 0) {
		EPRINTF("system(%s) failed.\n", cmd);
		goto ERR_EXIT;
	}
	return fd;

ERR_EXIT:
	if (fd >= 0)
		close(fd);
	if (p)
		free(p);
	return -1;
}

int cleanup_swapfile(char *path)
{
	unlink(path);
	return 0;
}

#if 0
/*
 * Check max nodes from /sys/devices/system/node/node* files (for NUMA)
 */
int get_max_nodes(void)
{
	/* We assume that there is only one node */
	return 1;
}
#endif

/*
 * Get unexist pid
 */
pid_t get_unexist_pid(void)
{
	pid_t pid;
	int st;

	pid = fork();
	switch (pid) {
	case -1:
		EPRINTF("fork failed.\n");
		return -1;
	case 0:
		_exit(0);
	default:
		wait(&st);
		return pid;
	}
}
