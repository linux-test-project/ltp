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

 * This file will include user space functions that will drive
 * the kernel module tpci to test various pci functions
 * and kernel calls. Each function will need to setup the tif
 * structure so that the in parameters and out parameters
 * are correctly initialized
 *
 * use tif structure for passing params between user
 * space and kernel space, in some tests it is really
 * not needed but makes easy to maintain all tests if
 * have the same process to read in params in the
 * kernel module no matter what the test is
 *
 * author: Sean Ruyle (srruyle@us.ibm.com)
 * date:   5/21/2003
 *
 * tpci_ki.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "../tpci/tpci.h"

int ki_generic(int fd, int flag) {
        int                     rc;
        tpci_interface_t        tif;

        /*
         * build interface structure
         */
        tif.in_len = 0;
        tif.in_data = 0;
        tif.out_len = 0;
        tif.out_data = 0;
        tif.out_rc = 0;

        /*
         * ioctl call for flag
         */
        rc = ioctl(fd, flag, &tif);
        if (rc) {
                printf("Ioctl error\n");
                return rc;
        }
	if (tif.out_rc) {
		printf("Specific errorr: ");
		return tif.out_rc;
	}

        return rc;
}

#if 0
int ki_probe_pci_dev(int fd) {

	int 			rc;
	tpci_interface_t	tif;

	/*
	 * build interface structure
	 */
	tif.in_len = 0;
	tif.in_data = 0;
	tif.out_len = 0;
	tif.out_data = 0;
	tif.out_rc = 0;

	/*
	 * ioctl call for PCI_PROBE
	 */
	rc = ioctl(fd, PCI_PROBE, &tif);
	if (rc) {
		printf("Ioctl error\n");
		return rc;
	}
	if (tif.out_rc) {
		printf("Specific error in ioctl call\n");
		return tif.out_rc;
	}

	return rc;
}

int ki_enable_pci(int fd) {

	int 			rc;
	tpci_interface_t	tif;

	/*
	 * build interface structure
	 */
	tif.in_len = 0;
        tif.in_data =0;
        tif.out_len = 0;
        tif.out_data = 0;
        tif.out_rc = 0;

	/*
	 * ioctl call for PCI_ENABLE
	 */
	rc = ioctl(fd, PCI_ENABLE, &tif);
	if (rc) {
                printf("Ioctl error\n");
                return rc;
        }
        if (tif.out_rc) {
                printf("Specific error in ioctl call\n");
                return tif.out_rc;
        }

        return rc;
}

int ki_disable_pci(int fd) {

	int 			rc;
	tpci_interface_t	tif;

	/*
	 * build interface structure
	 */
	tif.in_len = 0;
        tif.in_data =0;
        tif.out_len = 0;
        tif.out_data = 0;
        tif.out_rc = 0;

	/*
	 * ioctl call for PCI_DISABLE
	 */
	rc = ioctl(fd, PCI_DISABLE, &tif);
	if (rc) {
		printf("Ioctl error\n");
		return rc;
	}
	if (tif.out_rc) {
		printf("Specific error in ioctl call\n");
		return tif.out_rc;
	}

	return rc;
}

int ki_find_bus(int fd) {

        int                     rc;
        tpci_interface_t        tif;

        /*
         * build interface structure
         */
        tif.in_len = 0;
        tif.in_data =0;
        tif.out_len = 0;
        tif.out_data = 0;
        tif.out_rc = 0;

        /*
         * ioctl call for PCI_DISABLE
         */
        rc = ioctl(fd, FIND_BUS, &tif);
        if (rc) {
                printf("Ioctl error\n");
                return rc;
        }
        if (tif.out_rc) {
                printf("Specific error in ioctl call\n");
                return tif.out_rc;
        }

        return rc;
}

#endif