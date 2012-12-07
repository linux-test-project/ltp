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
 *
 * This file will include functions that will drive the
 * kernel module tusb to test various usb functions
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
 * date:   6/4/2003
 *
 * tusb_ki.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "../tusb/tusb.h"

/*
 * this generic function can be used
 * for any test calls that need no
 * additional setup beyond the normal
 * for making the ioctl call,
 * if the additional setup, ie: input
 * or output values are needed, set
 * tif in_data and out_data values
 * along with the corresponding length
 * in a more specific function
 */
int ki_generic(int fd, int flag)
{
	int rc;
	tusb_interface_t tif;

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
