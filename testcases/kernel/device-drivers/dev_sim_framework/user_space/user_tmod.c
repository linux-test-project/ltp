/*
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *

 * This is the main of your user space test program,
 * which will open the correct kernel module, find the
 * file descriptor value and use that value to make
 * ioctl calls to the system
 *
 * Use the ki_generic and other ki_testname functions
 * to abstract the calls from the main
 *
 * author: Sean Ruyle
 * date:   06/11/2003
 *
 * update: Marty Ridgeway
 * date:   09/02/2003
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/kernel.h>
#include <linux/errno.h>

#include "user_tmod.h"
#include "../kernel_space/tmod.h"

static int tmod_fd = -1;	/* file descriptor */

int tmodopen()
{

	dev_t devt;
	struct stat st;
	int rc = 0;

	devt = makedev(TMOD_MAJOR, 0);

	if (rc) {
		if (errno == ENOENT) {
			/* dev node does not exist. */
			rc = mkdir(DEVICE_NAME, (S_IFDIR | S_IRWXU |
						 S_IRGRP | S_IXGRP |
						 S_IROTH | S_IXOTH));
		} else {
			printf
			    ("ERROR: Problem with Base dev directory.  Error code from stat() is %d\n\n",
			     errno);
		}

	} else {
		if (!(st.st_mode & S_IFDIR)) {
			rc = unlink(DEVICE_NAME);
			if (!rc) {
				rc = mkdir(DEVICE_NAME, (S_IFDIR | S_IRWXU |
							 S_IRGRP | S_IXGRP |
							 S_IROTH | S_IXOTH));
			}
		}
	}

	/*
	 * Check for the /dev/tmod node, and create if it does not
	 * exist.
	 */
	rc = stat(DEVICE_NAME, &st);
	if (rc) {
		if (errno == ENOENT) {
			/* dev node does not exist */
			rc = mknod(DEVICE_NAME,
				   (S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP |
				    S_IWGRP), devt);
		} else {
			printf
			    ("ERROR:Problem with tbase device node directory.  Error code form stat() is %d\n\n",
			     errno);
		}

	} else {
		/*
		 * /dev/tbase CHR device exists.  Check to make sure it is for a
		 * block device and that it has the right major and minor.
		 */
		if ((!(st.st_mode & S_IFCHR)) || (st.st_rdev != devt)) {

			/* Recreate the dev node. */
			rc = unlink(DEVICE_NAME);
			if (!rc) {
				rc = mknod(DEVICE_NAME,
					   (S_IFCHR | S_IRUSR | S_IWUSR |
					    S_IRGRP | S_IWGRP), devt);
			}
		}
	}

	tmod_fd = open(DEVICE_NAME, O_RDWR);

	if (tmod_fd < 0) {
		printf("ERROR: Open of device %s failed %d errno = %d\n",
		       DEVICE_NAME, tmod_fd, errno);
		return errno;
	} else {
		printf("Device opened successfully \n");
		return 0;
	}

}

int tmodclose()
{

	if (tmod_fd != -1) {
		close(tmod_fd);
		tmod_fd = -1;
	}

	return 0;
}

int main()
{
	int rc;

	/* open the module */
	rc = tmodopen();
	if (rc) {
		printf("Test MOD Driver may not be loaded\n");
		exit(1);
	}

	/* make test calls */
	if (ki_generic(tmod_fd, LTP_OPTION1))
		printf("Failed on option 1 test\n");
	else
		printf("Success on option 1 test\n");

	/* close the module */
	rc = tmodclose();
	if (rc) {
		printf("Test MOD Driver may not be closed\n");
		exit(1);
	}

	return 0;
}
