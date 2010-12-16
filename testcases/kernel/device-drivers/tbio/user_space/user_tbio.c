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
 *
 *
 * This is the main of your user space test program,
 * which will open the correct kernel bioule, find the
 * file descriptor value and use that value to make
 * ioctl calls to the system
 *
 * Use the ki_generic and other ki_testname functions
 * to abstract the calls from the main
 *
 * bioule: tbio
 *  Copyright (c) International Business Machines  Corp., 2003
 *
 *  This program is free software;  you can redistribute it and/or bioify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *  the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;
 *
 *  FILE        : user_tbio.c
 *  USAGE       : kernel_space:./load_tbio.sh
 *                user_space  :./test_bio
 *
 *  DESCRIPTION : The bioule will test block i/o layer for kernel 2.5
 *  REQUIREMENTS:
 *                1) glibc 2.1.91 or above.
 *
 *  HISTORY     :
 *      11/19/2003 Kai Zhao (ltcd3@cn.ibm.com)
 *
 *  CODE COVERAGE: 74.9% - fs/bio.c (Total Coverage)
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/kernel.h>

#include "user_tbio.h"
#include "../kernel_space/tbio.h"

static int tbio_fd = -1;		/* file descriptor */

int
tbioopen() {

    dev_t devt;
	struct stat     st;
    int    rc = 0;

    devt = makedev(TBIO_MAJOR, 0);

    if (rc) {
        if (errno == ENOENT) {
            /* dev node does not exist. */
            rc = mkdir(DEVICE_NAME, (S_IFDIR | S_IRWXU |
                                                S_IRGRP | S_IXGRP |
                                                S_IROTH | S_IXOTH));
        } else {
            printf("ERROR: Problem with Base dev directory.  Error code from stat() is %d\n\n", errno);
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
            rc = mknod(DEVICE_NAME, (S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP), devt);
        } else {
            printf("ERROR:Problem with tbase device node directory.  Error code form stat() is %d\n\n", errno);
        }

    } else {
        /*
         * /dev/tbio CHR device exists.  Check to make sure it is for a
         * block device and that it has the right major and minor.
         */
        if ((!(st.st_mode & S_IFCHR)) ||
             (st.st_rdev != devt)) {

            /* Recreate the dev node. */
            rc = unlink(DEVICE_NAME);
            if (!rc) {
                rc = mknod(DEVICE_NAME, (S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP), devt);
            }
        }
    }

    tbio_fd = open(DEVICE_NAME, O_RDWR);

    if (tbio_fd < 0) {
        printf("ERROR: Open of device %s failed %d errno = %d\n", DEVICE_NAME,tbio_fd, errno);
        return errno;
    }
    else {
        printf("Device opened successfully \n");
      return 0;
    }

}

int
tbioclose() {

	if (tbio_fd != -1) {
		close (tbio_fd);
		tbio_fd = -1;
	}

	return 0;
}

int tbio_to_dev(int fd  , int flag)
{
	int rc;
	tbio_interface_t bif;

	memset(&bif , 0 , sizeof(tbio_interface_t));
	rc = posix_memalign(&bif.data , 512 , 1024);
	if (rc) {
		printf("posix_memalign failed\n");
		return -1;
	}

	strcpy(bif.data , "User space data");
	bif.data_len = 1024;
	bif.direction = TBIO_TO_DEV;
	bif.cmd = (char *) malloc (6);
	if (bif.cmd == NULL) {
		printf("malloc cmd space failed\n");
		free (bif.data);
		return -1;
	}
	strcpy(bif.cmd , "WRITE");
	bif.cmd_len = 6;

	rc = ioctl(fd, flag, &bif);
	if (rc) {
		free(bif.data);
		free(bif.cmd);
		printf("Ioctl error for TBIO_TO_DEV\n");
		return rc;
	}

	free(bif.data);
	free(bif.cmd);

	return 0;

}

int tbio_from_dev(int fd  , int flag)
{
	int rc;
	tbio_interface_t bif;

	memset(&bif , 0 , sizeof(tbio_interface_t));
	rc = posix_memalign(&bif.data , 512 , 1024);
	if (rc) {
		printf("posix_memalign failed\n");
		return -1;
	}

	memset(bif.data , 0 , 1024);

	bif.data_len = 1024;
	bif.direction = TBIO_FROM_DEV;
	bif.cmd = (char *) malloc (6);
	if (bif.cmd == NULL) {
		printf("malloc cmd space failed\n");
		free (bif.data);
		return -1;
	}
	strcpy(bif.cmd , "READ");
	bif.cmd_len = 6;

	rc = ioctl(fd, flag, &bif);
	if (rc) {
		free(bif.data);
		free(bif.cmd);
		printf("Ioctl error for TBIO_TO_DEV\n");
		return rc;
	}
	//printf("read from dev %s\n",bif.data);
	if (strcmp(bif.data , "User space data")) {
		printf("TBIO_FROM_DEV failed\n");
		free(bif.data);
		free(bif.cmd);
		return -1;
	}

	free(bif.data);
	free(bif.cmd);

	return 0;

}

int tbio_split_to_dev(int fd  , int flag)
{
	int rc;
	tbio_interface_t bif;

	memset(&bif , 0 , sizeof(tbio_interface_t));
	rc = posix_memalign(&bif.data , 512 , 2048);
	if (rc) {
		printf("posix_memalign failed\n");
		return -1;
	}

	strcpy(bif.data , "User space data");
	bif.data_len = 2048;
	bif.direction = TBIO_TO_DEV;
	bif.cmd = (char *) malloc (6);
	if (bif.cmd == NULL) {
		printf("malloc cmd space failed\n");
		free (bif.data);
		return -1;
	}
	strcpy(bif.cmd , "WRITE");
	bif.cmd_len = 6;

	rc = ioctl(fd, flag, &bif);
	if (rc) {
		free(bif.data);
		free(bif.cmd);
		printf("Ioctl error for TBIO_TO_DEV\n");
		return rc;
	}

	free(bif.data);
	free(bif.cmd);

	return 0;

}

int main()
{
	int rc;

	/* open the bioule */
	rc = tbioopen();
	if (rc) {
		printf("Test bio Driver may not be loaded\n");
		exit(1);
	}

	if (ki_generic(tbio_fd , LTP_TBIO_ALLOC))
		printf("Failed on LTP_TBIO_ALLOC test\n");
	else
		printf("Success on LTP_TBIO_ALLOC test\n");

	if (ki_generic(tbio_fd , LTP_TBIO_CLONE))
		printf("Failed on LTP_TBIO_CLONE test\n");
	else
		printf("Success on LTP_TBIO_CLONE test\n");

	if (ki_generic(tbio_fd , LTP_TBIO_GET_NR_VECS))
		printf("Failed on LTP_TBIO_GET_NR_VECS test\n");
	else
		printf("Success on LTP_TBIO_GET_NR_VECS test\n");

	if (ki_generic(tbio_fd , LTP_TBIO_ADD_PAGE))
		printf("Failed on LTP_TBIO_ADD_PAGE test\n");
	else
		printf("Success on LTP_TBIO_ADD_PAGE test\n");

	if (tbio_split_to_dev(tbio_fd , LTP_TBIO_SPLIT))
		printf("Failed on LTP_TBIO_SPLIT:write to dev\n");
	else
		printf("Success on LTP_TBIO_SPLIT:write to dev\n");

	if (tbio_to_dev(tbio_fd , LTP_TBIO_DO_IO))
		printf("Failed on LTP_TBIO_DO_IO:write to dev\n");
	else
		printf("Success on LTP_TBIO_DO_IO:write to dev\n");

	if (tbio_from_dev(tbio_fd , LTP_TBIO_DO_IO))
		printf("Failed on LTP_TBIO_DO_IO:read from dev\n");
	else
		printf("Success on LTP_TBIO_DO_IO:read from dev\n");

	if (ki_generic(tbio_fd , LTP_TBIO_PUT))
		printf("Failed on LTP_TBIO_PUT test\n");
	else
		printf("Success on LTP_TBIO_PUT test\n");

	/* close the bioule */
	rc = tbioclose();
	if (rc) {
                printf("Test bio Driver may not be closed\n");
                exit(1);
        }

      return 0;
}