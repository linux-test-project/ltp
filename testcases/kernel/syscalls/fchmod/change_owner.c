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
 *
 *		This function exit with 0 or 1 depending upon the
 *		success/failure of chown(2) system call.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	struct passwd *ltpuser;	/* password struct for ltpuser2 */
	struct group *ltpgroup;	/* group struct for ltpuser2 */
	uid_t user_uid;		/* user id of ltpuser2 */
	gid_t group_gid;	/* group id of ltpuser2 */
	char *test_name;	/* test specific name */
	char *path_name;	/* name of test directory/file */

	if (argc != 3) {
		fprintf(stderr,
			"This is a helper binary meant for internal LTP usage only\n");
		exit(1);
	}

	test_name = argv[1];
	path_name = argv[2];

	/*
	 * Get the user id and group id of "ltpuser2" user from password
	 * and group files.
	 */
	if ((ltpuser = getpwnam("nobody")) == NULL) {
		perror("change_owner: nobody not found in /etc/passwd");
		exit(1);
	}
	if ((ltpgroup = getgrnam("nobody")) == NULL) {
		if ((ltpgroup = getgrnam("nogroup")) == NULL) {
			perror
			    ("change_owner: nobody/nogroup's group not found in /etc/group");
			exit(1);
		}
	}

	user_uid = 0;
	group_gid = 0;

	/* Check for test specific name and set uid/gid accordingly */
	if (!(strcmp(test_name, "fchown03"))) {
		user_uid = -1;
		group_gid = ltpgroup->gr_gid;
	} else if (!(strcmp(test_name, "fchown04"))) {
		user_uid = ltpuser->pw_uid;
		group_gid = ltpgroup->gr_gid;
	}

	/*
	 * Change the ownership of test directory/file specified by
	 * pathname to that of user_uid and group_gid.
	 */
	if (chown(path_name, user_uid, group_gid) < 0) {
		fprintf(stderr, "change_owner: chown() of %s failed, error "
			"%d\n", path_name, errno);
		exit(1);
	}

	exit(0);
}
