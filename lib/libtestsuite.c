/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	libtestsuite.c
 *
 * DESCRIPTION
 *	file containing generic routines which are used by some of the LTP
 *	testsuite tests. Currently, the following routines are present in
 *	this library:
 *
 *	my_getpwnam(), do_file_setup()
 *
 * HISTORY
 *      11/03/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *      - Add the following functions to synchronise father and sons processes
 *      sync_pipe_create(), sync_pipe_wait(), sync_pipe_notify()
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>

#include "libtestsuite.h"
#include "test.h"
#include "usctest.h"

struct passwd *
my_getpwnam(char *name)
{
	struct passwd *saved_pwent;
	struct passwd *pwent;

	if ((pwent = getpwnam(name)) == NULL) {
		perror("getpwnam");
		tst_brkm(TBROK, NULL, "getpwnam() failed");
	}
	saved_pwent = (struct passwd *)malloc(sizeof(struct passwd));

	*saved_pwent = *pwent;

	return(saved_pwent);
}

void
do_file_setup(char *fname)
{
	int fd;

	if ((fd = open(fname,O_RDWR|O_CREAT,0700)) == -1) {
		tst_resm(TBROK, "open(%s, O_RDWR|O_CREAT,0700) Failed, "
			 "errno=%d : %s", fname, errno, strerror(errno));
	}

	if (close(fd) == -1) {
		tst_resm(TWARN, "close(%s) Failed on file create, errno=%d : "
			 "%s", fname, errno, strerror(errno));
	}
}

static char pipe_name[256];

void generate_pipe_name(const char *name)
{
	char *p;

	p = strrchr(name, '/');
	if (p == NULL)
		p = (char *) name;
	else
		p++;
	snprintf(pipe_name, 255, "%s/ltp_fifo_%s", P_tmpdir, p);
}

int sync_pipe_create(int fd[], const char *name)
{
	int ret;

	if (name != NULL) {
		generate_pipe_name(name);

		ret = open(pipe_name, O_RDWR);
		if (ret != -1) {
			fd[0] = ret;
			fd[1] = open(pipe_name, O_RDWR);
		} else {
			ret = mkfifo(pipe_name, S_IRWXU);
			if (!ret) {
				fd[0] = open(pipe_name, O_RDWR);
				fd[1] = open(pipe_name, O_RDWR);
			}
		}
		return ret;
	} else
		return pipe(fd);
}

int sync_pipe_close(int fd[], const char *name)
{
	int r = 0;

	if (fd[0] != -1)
		r = close (fd[0]);
	if (fd[1] != -1)
		r |= close (fd[1]);

	if (name != NULL) {
		generate_pipe_name(name);
		unlink(pipe_name);
	}
	return r;
}

int sync_pipe_wait(int fd[])
{
	char buf;
	int r;

	if (fd[1] != -1) {
		close (fd[1]);
		fd[1] = -1;
	}

	r = read (fd[0], &buf, 1);

	if ((r != 1) || (buf != 'A'))
		return -1;
	return 0;
}

int sync_pipe_notify(int fd[])
{
	char buf = 'A';
	int r;

	if (fd[0] != -1) {
		close (fd[0]);
		fd[0] = -1;
	}

	r = write (fd[1], &buf, 1);

	if (r != 1)
		return -1;
	return 0;
}
