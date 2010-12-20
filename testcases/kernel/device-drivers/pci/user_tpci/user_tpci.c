#include <stdio.h>
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include "user_tpci.h"
#include "../tpci/tpci.h"

static int tpci_fd = -1;	/* file descriptor */

int
tpciopen() {

    dev_t devt;
	struct stat     st;
    int    rc = 0;

    devt = makedev(TPCI_MAJOR, 0);

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
         * /dev/tbase CHR device exists.  Check to make sure it is for a
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

    tpci_fd = open(DEVICE_NAME, O_RDWR);

    if (tpci_fd < 0) {
        printf("ERROR: Open of device %s failed %d errno = %d\n", DEVICE_NAME,tpci_fd, errno);
        return errno;
    }
    else {
        printf("Device opened successfully \n");
      return 0;
    }

}

int
tpciclose() {

	if (tpci_fd != -1) {
		close (tpci_fd);
		tpci_fd = -1;
	}

	return 0;
}

int main() {
	int rc;

	rc = tpciopen();
	if (rc) {
		printf("Test PCI Driver may not be loaded\n");
		exit(1);
	}

	/* test find pci */
	if (ki_generic(tpci_fd, PCI_PROBE)) {
		printf("Failed to find a pci device\n");
		exit(1);
	}
	else
		printf("Success probing for pci device\n");

	/* test disable device */
	if (ki_generic(tpci_fd, PCI_DISABLE))
		printf("Failed to disable device \nMay still be in use by system\n");
	else
		printf("Disabled device\n");

	/* test enable device */
	if (ki_generic(tpci_fd, PCI_ENABLE))
		printf("Failed to enable device\n");
	else
		printf("Enabled device\n");

	/* test find from bus */
	if (ki_generic(tpci_fd, FIND_BUS))
                printf("Failed to find from bus pointer\n");
        else
                printf("Found device from bus pointer\n");

	/* test find from device */
	if (ki_generic(tpci_fd, FIND_DEVICE))
                printf("Failed to find device from device info\n");
        else
                printf("Found device from device info\n");

	/* test find from class */
	if (ki_generic(tpci_fd, FIND_CLASS))
                printf("Failed to find device from class\n");
        else
                printf("Found device from class \n");

	/* test find subsys */
	if (ki_generic(tpci_fd, FIND_SUBSYS))
                printf("Failed to find device from subsys info\n");
        else
                printf("Found device from subsys info\n");

	/* test scan bus */
	if (ki_generic(tpci_fd, BUS_SCAN))
                printf("Failed on bus scan call\n");
        else
                printf("Success scanning bus\n");

	/* test scan slot */
	if (ki_generic(tpci_fd, SLOT_SCAN))
                printf("Failed on scan slot \n");
        else
                printf("Success scan slot\n");

	/* test enable bridges */
	if (ki_generic(tpci_fd, ENABLE_BRIDGES))
                printf("Failed to enable bridges\n");
        else
                printf("Enabled bridges\n");

	/* test bus add devices */
	if (ki_generic(tpci_fd, BUS_ADD_DEVICES))
                printf("Failed on bus add devices call\n");
        else
                printf("Success bus add devices\n");

	/* test match device */
	if (ki_generic(tpci_fd, MATCH_DEVICE))
		printf("Failed on match device call\n");
	else
		printf("Success match device\n");

#if 0
	/* test unregister driver */
	if (ki_generic(tpci_fd, UNREG_DRIVER))
                printf("Failed to unregister driver\n");
        else
                printf("Unregistered driver\n");
#endif

	/* test register driver */
	if (ki_generic(tpci_fd, REG_DRIVER))
                printf("Failed to register driver\n");
        else
                printf("Registerd driver\n");

	/* test pci resources */
	if (ki_generic(tpci_fd, PCI_RESOURCES))
                printf("Failed on pci_resources call\n");
        else
                printf("Success pci resources\n");

	/* test save state */
	if (ki_generic(tpci_fd, SAVE_STATE))
		printf("Failed to save state of device\n");
	else
		printf("Saved state of device\n");

	/* test restore state */
	if (ki_generic(tpci_fd, RESTORE_STATE))
		printf("Failed to restore state\n");
	else
		printf("Restored state\n");

	/* test max bus */
        if (ki_generic(tpci_fd, TEST_MAX_BUS))
                printf("Failed on max bus call\n");
        else
                printf("Success max bus \n");

	if (ki_generic(tpci_fd, FIND_CAP))
		printf("Does not have tested capability\n");
	else
		printf("Device has tested capability\n");

	rc = tpciclose();
	if (rc) {
                printf("Test PCI Driver may not be closed\n");
                exit(1);
        }

	return 0;
}
