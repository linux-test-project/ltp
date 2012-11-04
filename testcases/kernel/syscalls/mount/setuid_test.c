/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*
 * Description: This is a setuid to root program invoked by a non-root
 *              process to validate the mount flag MS_NOSUID.
 *
 *              This function exit with 0 or 1 depending upon the
 *              success/failure of setuid(2) system call.
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>
#include <errno.h>

/* Save the effective and real UIDs. */

static uid_t ruid;

/* Restore the effective UID to its original value. */

int do_setuid(void)
{
	int status;

	status = setreuid(ruid, 0);
	if (status < 0) {
		return 1;
	} else {
		return 0;
	}
	return 0;
}

/* Main program. */

int main(void)
{
	int exit_status;

	/* Save the real and effective user IDs.  */
	ruid = getuid();
	exit_status = do_setuid();

	exit(exit_status);
}
