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
 */
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include "../tusb/tusb.h"

static int tusb_fd = -1;	//file descriptor

int tusbopen()
{

	dev_t devt;
	struct stat st;
	int rc = 0;

	devt = makedev(TUSB_MAJOR, 0);

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
	 * Check for the /dev/tbase node, and create if it does not
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

	tusb_fd = open(DEVICE_NAME, O_RDWR);

	if (tusb_fd < 0) {
		printf("ERROR: Open of device %s failed %d errno = %d\n",
		       DEVICE_NAME, tusb_fd, errno);
		return errno;
	} else {
		printf("Device opened successfully \n");
		return 0;
	}

}

int tusbclose()
{
	/*
	 * Close the tusb driver
	 */
	if (tusb_fd != -1) {
		close(tusb_fd);
		tusb_fd = -1;
	}

	return 0;
}

int main()
{
	int rc = 0;

	rc = tusbopen();
	if (rc) {
		printf("tusb driver may not be loaded\n");
		exit(1);
	}

	/* test find device pointer */
	if (ki_generic(tusb_fd, FIND_DEV))
		printf("Failed to find usb device pointer\n");
	else
		printf("Found usb device pointer\n");

	/* test find usb hostcontroller */
	if (ki_generic(tusb_fd, TEST_FIND_HCD))
		printf("Failed to find usb hcd pointer\n");
	else
		printf("Found usb hcd pointer\n");

	/* test hcd probe */
	if (ki_generic(tusb_fd, TEST_HCD_PROBE))
		printf("Failed on hcd probe call\n");
	else
		printf("Success hcd probe\n");

	/* test hcd suspend */
	if (ki_generic(tusb_fd, TEST_HCD_SUSPEND))
		printf("Failed on hcd suspend call\n");
	else
		printf("Success hcd suspend\n");

	/* test hcd resume */
	if (ki_generic(tusb_fd, TEST_HCD_RESUME))
		printf("Failed on hcd resume call\n");
	else
		printf("Success hcd resume\n");

#if 0
	/* test hcd remove */
	if (ki_generic(tusb_fd, TEST_HCD_REMOVE))
		printf("Failed on hcd remove call\n");
	else
		printf("Success hcd remove\n");
#endif

	tusbclose();

	tst_exit();
}
