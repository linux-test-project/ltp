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
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>

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
