/*
* Copyright (c) International Business Machines Corp., 2007
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
***************************************************************************/
#include "libclone.h"

int do_clone_tests(unsigned long clone_flags,
		   int (*fn1) (void *arg), void *arg1,
		   int (*fn2) (void *arg), void *arg2)
{
	int ret;

	ret = ltp_clone_quick(clone_flags | SIGCHLD, fn1, arg1);

	if (ret == -1) {
		return -1;
	}
	if (fn2)
		ret = fn2(arg2);
	else
		ret = 0;

	return ret;
}

int do_unshare_tests(unsigned long clone_flags,
		     int (*fn1) (void *arg), void *arg1,
		     int (*fn2) (void *arg), void *arg2)
{
	int pid, ret = 0;
	int retpipe[2];
	char buf[2];

	if (pipe(retpipe) == -1) {
		perror("pipe");
		return -1;
	}
	pid = fork();
	if (pid == -1) {
		perror("fork");
		close(retpipe[0]);
		close(retpipe[1]);
		return -1;
	}
	if (pid == 0) {
		close(retpipe[0]);
		ret = ltp_syscall(SYS_unshare, clone_flags);
		if (ret == -1) {
			if (write(retpipe[1], "0", 2) < 0) {
				perror("unshare:write(retpipe[1], ..)");
			}
			close(retpipe[1]);
			exit(1);
		} else {
			if (write(retpipe[1], "1", 2) < 0) {
				perror("unshare:write(retpipe[1], ..)");
			}
		}
		close(retpipe[1]);
		ret = fn1(arg1);
		exit(ret);
	} else {
		close(retpipe[1]);
		if (read(retpipe[0], &buf, 2) < 0) {
			perror("unshare:read(retpipe[0], ..)");
		}
		close(retpipe[0]);
		if (*buf == '0')
			return -1;
		if (fn2)
			ret = fn2(arg2);
	}

	return ret;
}

int do_plain_tests(int (*fn1) (void *arg), void *arg1,
		   int (*fn2) (void *arg), void *arg2)
{
	int ret = 0, pid;

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
	}
	if (pid == 0)
		exit(fn1(arg1));
	if (fn2)
		ret = fn2(arg2);
	return ret;
}

int do_clone_unshare_test(int use_clone, unsigned long clone_flags,
			  int (*fn1) (void *arg), void *arg1)
{
	switch (use_clone) {
	case T_NONE:
		return do_plain_tests(fn1, arg1, NULL, NULL);
	case T_CLONE:
		return do_clone_tests(clone_flags, fn1, arg1, NULL, NULL);
	case T_UNSHARE:
		return do_unshare_tests(clone_flags, fn1, arg1, NULL, NULL);
	default:
		printf("%s: bad use_clone option: %d\n", __FUNCTION__,
		       use_clone);
		return -1;
	}
}

/*
 * Run fn1 in a unshared environmnent, and fn2 in the original context
 */
int do_clone_unshare_tests(int use_clone, unsigned long clone_flags,
			   int (*fn1) (void *arg), void *arg1,
			   int (*fn2) (void *arg), void *arg2)
{
	switch (use_clone) {
	case T_NONE:
		return do_plain_tests(fn1, arg1, fn2, arg2);
	case T_CLONE:
		return do_clone_tests(clone_flags, fn1, arg1, fn2, arg2);
	case T_UNSHARE:
		return do_unshare_tests(clone_flags, fn1, arg1, fn2, arg2);
	default:
		printf("%s: bad use_clone option: %d\n", __FUNCTION__,
		       use_clone);
		return -1;
	}
}
