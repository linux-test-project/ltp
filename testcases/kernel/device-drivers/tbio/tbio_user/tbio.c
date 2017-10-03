/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This is the main of your user space test program,
 * which will open the correct kernel bioule, find the
 * file descriptor value and use that value to make
 * ioctl calls to the system
 *
 * Use the ki_generic and other ki_testname functions
 * to abstract the calls from the main
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
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <linux/kernel.h>
#include <unistd.h>
#include <string.h>

#include "test.h"
#include "safe_macros.h"
#include "old_module.h"

#include "../tbio_kernel/tbio.h"

char *TCID = TBIO_DEVICE_NAME;

static const char module_name[]	= "ltp_tbio.ko";
static int module_loaded;
static int tbio_fd = -1;

void cleanup(void)
{

	if (tbio_fd != -1) {
		close(tbio_fd);
		tbio_fd = -1;
	}

	if (module_loaded)
		tst_module_unload(NULL, module_name);

	if (unlink(DEVICE_NAME) && (errno != ENOENT))
		tst_brkm(TBROK | TERRNO, NULL, "unlink failed");
}


void setup(void)
{
	dev_t devt;
	struct stat st;
	unsigned int i, valid_node_created;

	tst_require_root();

	if (tst_kvercmp(2, 6, 0) < 0) {
		tst_brkm(TCONF, NULL,
			"Test must be run with kernel 2.6 or newer");
	}

	tst_module_load(cleanup, module_name, NULL);
	module_loaded = 1;

	SAFE_FILE_SCANF(cleanup, "/sys/class/block/tbio/dev",
		"%d:0", &TBIO_MAJOR);

	devt = makedev(TBIO_MAJOR, 0);

	/*
	 * Wait until udev creates the device node.
	 * If the node is not created or invalid, create it manually.
	 */
	valid_node_created = 0;
	for (i = 0; i < 50; i++) {
		if (stat(DEVICE_NAME, &st)) {
			if (errno != ENOENT)
				tst_brkm(TBROK | TERRNO, cleanup,
					 "stat() failed");
		} else {
			if ((st.st_mode & S_IFBLK) && (st.st_rdev == devt)) {
				valid_node_created = 1;
				break;
			}
		}

		usleep(100000);
	}

	if (!valid_node_created) {
		tst_resm(TINFO,
			 "The device file was not created by udev, "
			 "proceeding with manual creation");

		if (unlink(DEVICE_NAME) && (errno != ENOENT))
			tst_brkm(TBROK | TERRNO, cleanup, "unlink() failed");
		if (mknod(DEVICE_NAME, S_IFBLK | S_IRUSR | S_IWUSR |
			  S_IRGRP | S_IWGRP, devt))
			tst_brkm(TBROK | TERRNO, cleanup, "mknod() failed");
	}

	tbio_fd = SAFE_OPEN(cleanup, DEVICE_NAME, O_RDWR);

	tst_resm(TINFO, "Device opened successfully ");
}


int tbio_to_dev(int fd, int flag)
{
	int rc;
	tbio_interface_t bif;

	memset(&bif, 0, sizeof(tbio_interface_t));
	rc = posix_memalign(&bif.data, 512, 1024);
	if (rc) {
		tst_resm(TINFO, "posix_memalign failed");
		return -1;
	}

	strcpy(bif.data, "User space data");
	bif.data_len = 1024;
	bif.direction = TBIO_TO_DEV;
	bif.cmd = SAFE_MALLOC(cleanup, 6);
	if (bif.cmd == NULL) {
		tst_resm(TINFO, "malloc cmd space failed");
		free(bif.data);
		return -1;
	}
	strcpy(bif.cmd, "WRITE");
	bif.cmd_len = 6;

	rc = ioctl(fd, flag, &bif);
	if (rc) {
		free(bif.data);
		free(bif.cmd);
		tst_resm(TINFO, "Ioctl error for TBIO_TO_DEV");
		return rc;
	}

	free(bif.data);
	free(bif.cmd);

	return 0;

}

int tbio_from_dev(int fd, int flag)
{
	int rc;
	tbio_interface_t bif;

	memset(&bif, 0, sizeof(tbio_interface_t));
	rc = posix_memalign(&bif.data, 512, 1024);
	if (rc) {
		tst_resm(TINFO, "posix_memalign failed");
		return -1;
	}

	memset(bif.data, 0, 1024);

	bif.data_len = 1024;
	bif.direction = TBIO_FROM_DEV;
	bif.cmd = SAFE_MALLOC(cleanup, 5);
	if (bif.cmd == NULL) {
		tst_resm(TINFO, "malloc cmd space failed");
		free(bif.data);
		return -1;
	}
	strcpy(bif.cmd, "READ");
	bif.cmd_len = 5;

	rc = ioctl(fd, flag, &bif);
	if (rc) {
		free(bif.data);
		free(bif.cmd);
		tst_resm(TINFO, "Ioctl error for TBIO_TO_DEV");
		return rc;
	}

	if (strcmp(bif.data, "User space data")) {
		tst_resm(TINFO, "TBIO_FROM_DEV failed");
		free(bif.data);
		free(bif.cmd);
		return -1;
	}

	free(bif.data);
	free(bif.cmd);

	return 0;

}

int tbio_split_to_dev(int fd, int flag)
{
	int rc;
	tbio_interface_t bif;

	memset(&bif, 0, sizeof(tbio_interface_t));
	rc = posix_memalign(&bif.data, 512, 2048);
	if (rc) {
		tst_resm(TINFO, "posix_memalign failed");
		return -1;
	}

	strcpy(bif.data, "User space data");
	bif.data_len = 2048;
	bif.direction = TBIO_TO_DEV;
	bif.cmd = SAFE_MALLOC(cleanup, 6);
	if (bif.cmd == NULL) {
		tst_resm(TINFO, "malloc cmd space failed");
		free(bif.data);
		return -1;
	}
	strcpy(bif.cmd, "WRITE");
	bif.cmd_len = 6;

	rc = ioctl(fd, flag, &bif);
	if (rc) {
		free(bif.data);
		free(bif.cmd);
		tst_resm(TINFO, "Ioctl error for TBIO_TO_DEV");
		return rc;
	}

	free(bif.data);
	free(bif.cmd);

	return 0;

}

int ki_generic(int fd, int flag)
{
	tbio_interface_t bif;

	int rc = ioctl(fd, flag, &bif);
	if (rc)
		tst_resm(TINFO | TERRNO, "ioctl error");

	return rc;
}


int main(void)
{
	setup();

	if (ki_generic(tbio_fd, LTP_TBIO_ALLOC))
		tst_resm(TFAIL, "failed on LTP_TBIO_ALLOC test");
	else
		tst_resm(TPASS, "success on LTP_TBIO_ALLOC test");

	if (ki_generic(tbio_fd, LTP_TBIO_CLONE))
		tst_resm(TFAIL, "failed on LTP_TBIO_CLONE test");
	else
		tst_resm(TPASS, "success on LTP_TBIO_CLONE test");

	if (ki_generic(tbio_fd, LTP_TBIO_GET_NR_VECS))
		tst_resm(TFAIL, "failed on LTP_TBIO_GET_NR_VECS test");
	else
		tst_resm(TPASS, "success on LTP_TBIO_GET_NR_VECS test");

	if (ki_generic(tbio_fd, LTP_TBIO_ADD_PAGE))
		tst_resm(TFAIL, "failed on LTP_TBIO_ADD_PAGE test");
	else
		tst_resm(TPASS, "success on LTP_TBIO_ADD_PAGE test");

	if (tbio_split_to_dev(tbio_fd, LTP_TBIO_SPLIT))
		tst_resm(TFAIL, "failed on LTP_TBIO_SPLIT:write to dev");
	else
		tst_resm(TPASS, "success on LTP_TBIO_SPLIT:write to dev");

	if (tbio_to_dev(tbio_fd, LTP_TBIO_DO_IO))
		tst_resm(TFAIL, "failed on LTP_TBIO_DO_IO:write to dev");
	else
		tst_resm(TPASS, "success on LTP_TBIO_DO_IO:write to dev");

	if (tbio_from_dev(tbio_fd, LTP_TBIO_DO_IO))
		tst_resm(TFAIL, "failed on LTP_TBIO_DO_IO:read from dev");
	else
		tst_resm(TPASS, "success on LTP_TBIO_DO_IO:read from dev");

	if (ki_generic(tbio_fd, LTP_TBIO_PUT))
		tst_resm(TFAIL, "failed on LTP_TBIO_PUT test");
	else
		tst_resm(TPASS, "success on LTP_TBIO_PUT test");

	cleanup();

	tst_exit();
}
