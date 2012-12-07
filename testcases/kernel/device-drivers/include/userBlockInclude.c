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
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/errno.h>

#include "includeTest.h"

int open_block_device(void);

int block_dev_handle = 0;	/* handle to INC test block device */

int main(int argc, char **argv)
{

	incdev_cmd_t cmd = { 0, 0 };
	int rc;

	rc = open_block_device();

	if (!rc) {

		block_dev_handle = open(DEVICE_NAME, O_RDWR);

		if (block_dev_handle < 0) {
			printf
			    ("ERROR: Open of device %s failed %d errno = %d\n",
			     DEVICE_NAME, block_dev_handle, errno);
		} else {
			rc = ioctl(block_dev_handle, INCDEV_CMD, &cmd);

			printf("return from ioctl %d \n", rc);
		}

	} else {
		printf("ERROR: Create/open block device failed\n");
	}

	return 0;

	printf("Block Include Test complete.\n");
}

int open_block_device()
{
	dev_t devt;
	struct stat statbuf;
	int rc;

	if (block_dev_handle == 0) {

		/* check for the /dev/ subdir, and create if it does not exist.
		 *
		 * If devfs is running and mounted on /dev, these checks will all pass,
		 * so a new node will not be created.
		 */

		devt = makedev(INCLUDEMAJOR, 0);

		rc = stat(INCLUDE_DEVICE_PATH, &statbuf);

		if (rc) {
			if (errno == ENOENT) {
				/* dev node does not exist. */
				rc = mkdir(INCLUDE_DEVICE_PATH,
					   (S_IFDIR | S_IRWXU | S_IRGRP |
					    S_IXGRP | S_IROTH | S_IXOTH));

			} else {
				printf("ERROR: Problem with INC dev directory.  Error code from stat(
			) is %d\n\n", errno);
			}
		} else {
			if (!(statbuf.st_mode & S_IFDIR)) {
				rc = unlink(INCLUDE_DEVICE_PATH);
				if (!rc) {
					rc = mkdir(INCLUDE_DEVICE_PATH,
						   (S_IFDIR | S_IRWXU | S_IRGRP
						    | S_IXGRP | S_IROTH |
						    S_IXOTH));
				}
			}
		}

		/*
		 * Check for the block_device node, and create if it does not
		 * exist.
		 */

		rc = stat(DEVICE_NAME, &statbuf);
		if (rc) {
			if (errno == ENOENT) {
				/* dev node does not exist */
				rc = mknod(DEVICE_NAME,
					   (S_IFBLK | S_IRUSR | S_IWUSR |
					    S_IRGRP | S_IWGRP), devt);
			} else {
				printf
				    ("ERROR:Problem with block device node directory.  Error code form stat() is %d\n\n",
				     errno);
			}

		} else {
			/*
			 * Device exists. Check to make sure it is for a
			 * block device and that it has the right major and minor.
			 */

			if ((!(statbuf.st_mode & S_IFBLK)) ||
			    (statbuf.st_rdev != devt)) {
				/* Recreate the dev node. */
				rc = unlink(DEVICE_NAME);
				if (!rc) {
					rc = mknod(DEVICE_NAME,
						   (S_IFBLK | S_IRUSR | S_IWUSR
						    | S_IRGRP | S_IWGRP), devt);
				}
			}
		}

	}
	return rc;
}
