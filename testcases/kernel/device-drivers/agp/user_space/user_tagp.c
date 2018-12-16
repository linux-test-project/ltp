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
/*
 * This is the main of your user space test program,
 * which will open the correct kernel module, find the
 * file descriptor value and use that value to make
 * ioctl calls to the system
 *
 * Use the ki_generic and other ki_testname functions
 * to abstract the calls from the main
 *
 * author: Kai Zhao
 * date:   08/25/2003
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

#include "user_tagp.h"
#include "../kernel_space/tagp.h"

static int tagp_fd = -1;	/* file descriptor */

int tagpopen()
{

	dev_t devt;
	struct stat st;
	int rc = 0;

	devt = makedev(TAGP_MAJOR, 0);

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

	tagp_fd = open(DEVICE_NAME, O_RDWR);

	if (tagp_fd < 0) {
		printf("ERROR: Open of device %s failed %d errno = %d\n",
		       DEVICE_NAME, tagp_fd, errno);
		return errno;
	} else {
		printf("Device opened successfully \n");
		return 0;
	}

}

int tagpclose()
{

	if (tagp_fd != -1) {
		close(tagp_fd);
		tagp_fd = -1;
	}

	return 0;
}

int agpgart_io_test()
{
	int tagp_fd = 0;
	char read_buf[BUFFER_LEN];

	if ((tagp_fd = open("/dev/agpgart", O_RDWR)) < 0) {
		printf("Open /dev/agpgart failed \n");
		return -1;
	}

	close(tagp_fd);

	return 0;
}

int main()
{
	int rc;

	if (agpgart_io_test())
		printf("Test agpgart io failed\n");
	else
		printf("Test agpgart io success\n");

	/* open the module */
	rc = tagpopen();
	if (rc) {
		printf("Test AGP Driver may not be loaded\n");
		exit(1);
	}

	/* make test calls for pci_find_device */
	if (ki_generic(tagp_fd, TEST_PCI_FIND_DEV))
		printf("Success: Expected failure for pci_find_dev test\n");
	else
		printf("Fail on pci_find_dev test\n");

	/* make test calls for agp_backend_acquier */
	if (ki_generic(tagp_fd, TEST_BACKEND_ACQUIRE))
		printf("Fail on agp_backend_acquier\n");
	else
		printf("Success on agp_backend_acquier\n");

	/* make test calls for agp_backend_release */
	if (ki_generic(tagp_fd, TEST_BACKEND_RELEASE))
		printf("Fail on agp_backend_release\n");
	else
		printf("Success on agp_backend_release\n");

	/* make test calls for agp_alloc_bridge */
	if (ki_generic(tagp_fd, TEST_ALLOC_BRIDGE))
		printf("Fail on agp_alloc_bridge \n");
	else
		printf("Success on agp_alloc_bridge\n");

	/* make test calls for and agp_put_bridge */
	if (ki_generic(tagp_fd, TEST_PUT_BRIDGE))
		printf("Fail on agp_put_bridge\n");
	else
		printf("Success on agp_put_bridge\n");

	/* make test calls for agp_create_memory and agp_free_memory */
	if (ki_generic(tagp_fd, TEST_CREATE_AND_FREE_MEMORY))
		printf("Fail on agp_create_memory \n");
	else
		printf("Success on agp_create_memory\n");
/*
	if (ki_generic(tagp_fd, TEST_FREE_MEMORY))
		printf("Fail on agp_free_memory\n");
	else
			printf("Success on agp_free_memory\n");
*////////////////////////////////////////////////////////////////////////
	/* make test calls for agp_num_entries */
	if (ki_generic(tagp_fd, TEST_NUM_ENTRIES))
		printf("Fail on agp_num_entries\n");
	else
		printf("Success on agp_num_entries\n");

	/* make test calls for agp_copy_info */
	if (ki_generic(tagp_fd, TEST_COPY_INFO))
		printf("Fail on agp_copy_info\n");
	else
		printf("Success on agp_copy_info\n");

	/* make test calls for agp_alloc_memory */
//      if (ki_generic(tagp_fd, TEST_ALLOC_MEMORY_AND_BAND_UNBAND))
//              printf("Fail on agp_alloc_memory_and_band_unband\n");
//      else
//              printf("Success on agp_alloc_memory_and_band_unband\n");

	/* make test calls for agp_get_version */
	if (ki_generic(tagp_fd, TEST_GET_VERSION))
		printf("Fail on agp_get_version\n");
	else
		printf("Success on agp_get_version\n");

	/* make test calls for agp_generic_enable */
	if (ki_generic(tagp_fd, TEST_GENERIC_ENABLE))
		printf("Fail on agp_generic_enable\n");
	else
		printf("Success on agp_generic_enable\n");

	/* make test calls for agp_generic_create_gatt_table */
	if (ki_generic(tagp_fd, TEST_GENERIC_CREATE_GATT_TABLE))
		printf("Fail on agp_generic_create_gatt_table\n");
	else
		printf("Success on agp_generic_create_gatt_table\n");

	/* make test calls for agp_generic_free_gatt_table */
	if (ki_generic(tagp_fd, TEST_GENERIC_FREE_GATT_TABLE))
		printf("Fail on agp_generic_free_gatt_table\n");
	else
		printf("Success on agp_generic_free_gatt_table\n");

	/* make test calls for agp_generic_insert_memory */
	if (ki_generic(tagp_fd, TEST_GENERIC_INSERT_MEMORY))
		printf("Fail on agp_generic_insert_memory\n");
	else
		printf("Success on agp_generic_insert_memory\n");

	/* make test calls for agp_generic_alloc_by_type */
	if (ki_generic(tagp_fd, TEST_GENERIC_ALLOC_BY_TYPE))
		printf("Fail on agp_generic_alloc_by_type\n");
	else
		printf("Success on agp_generic_alloc_by_type\n");

	/* make test calls for agp_generic_alloc_page */
	if (ki_generic(tagp_fd, TEST_GENERIC_ALLOC_PAGE))
		printf("Fail on agp_generic_alloc_page\n");
	else
		printf("Success on agp_generic_alloc_page\n");

	/* make test calls for agp_generic_destroy_page */
	if (ki_generic(tagp_fd, TEST_GENERIC_ALLOC_PAGE))
		printf("Fail on agp_generic_destroy_page\n");
	else
		printf("Success on agp_generic_destroy_page\n");

	/* make test calls for agp_enable */
	if (ki_generic(tagp_fd, TEST_ENABLE))
		printf("Fail on agp_enable\n");
	else
		printf("Success on agp_enable\n");

	/* make test calls for agp_global_cache_flush */
	if (ki_generic(tagp_fd, TEST_GLOBAL_CACHE_FLUSH))
		printf("Fail on agp_global_cache_flush\n");
	else
		printf("Success on agp_gloabl_cache_flush\n");

	/* make test calls for agp_generic_mask_memory */
	if (ki_generic(tagp_fd, TEST_GENERIC_MASK_MEMORY))
		printf("Fail on agp_generic_mask_memory\n");
	else
		printf("Success on agp_generic_mask_memory\n");

	/* close the module */
	rc = tagpclose();
	if (rc) {
		printf("Test AGP Driver may not be closed\n");
		exit(1);
	}

	return 0;
}
