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
 * Description: This is a setuid to root program invoked by a non-root
 *		process to change the user id/group id bits on the test
 *		directory/file created in the setup function.
 *		It sets uid to another non-root user and creates a
 *		symlink of testfile/directory.
 *
 *		This function exit with 0 or 1 depending upon the
 *		success/failure each system call.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <string.h>

#define LTPUSER		"bin"
#define LTPGRP		"bin"
#define SFILE1		"sfile_1"

int
main(int argc, char **argv)
{
	struct passwd *ltpuser;		/* password struct for nobody */
	struct group *ltpgroup;		/* group struct for nobody */
	uid_t user_uid;			/* user id of nobody */
	gid_t group_gid;		/* group id of nobody */
	char *path_name;		/* name of test directory/file */

	path_name = argv[1];

	/*
	 * Get the user id and group id of "ltpuser" user from password
	 * and group files.
	 */
	if ((ltpuser = getpwnam(LTPUSER)) == NULL) {
		fprintf(stderr, "change_owner: %s not found in /etc/passwd\n",
			LTPUSER);
		exit(1);
	}
	if ((ltpgroup = getgrnam(LTPGRP)) == NULL) {
		fprintf(stderr, "change_owner: %s not found in /etc/group\n",
			LTPGRP);
		exit(1);
	}
	user_uid = ltpuser->pw_uid;
	group_gid = ltpgroup->gr_gid;

	/*
	 * Change the ownership of test directory/file specified by
	 * pathname to that of LTPUSER user_uid and group_gid.
	 */
	if (chown(path_name, user_uid, group_gid) < 0) {
		fprintf(stderr, "change_owner: chown() of %s failed, "
			"error %d\n", path_name, errno);
		exit(1);
	}

	/* Set the process uid to that LTPUSER */
	if (setuid(user_uid) < 0) {
		fprintf(stderr, "change_owner: setuid() to %s fails, error:%d",
			LTPUSER, errno);
		exit(1);
	}

	/* Creat a symlink of testfile created above */
	if (symlink(path_name, SFILE1) < 0) {
		fprintf(stderr, "change_owner: symlink() of %s Failed, "
			"errno=%d : %s", path_name, errno, strerror(errno));
		exit(1);
	}

	exit(0);
}
