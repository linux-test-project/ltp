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
 * This file will include user space functions that will drive
 * the kernel module to test various functions and kernel
 * calls. Each function will need to setup the tif structure
 * so that the in parameters and out parameters are correctly
 * initialized
 *
 * use tif structure for passing params between user
 * space and kernel space, in some tests it is really not
 * needed, and if nothing is needed to pass in utilize
 * the ki_generic function below. the tif structure makes
 * it easy to maintain all the tests if they have the same
 * process in kernel space to read in params in the kernel
 * module no matter what the test is
 *
 * author: Kai Zhao
 * date:   08/25/2003
 *
 * tagp_ki.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "../kernel_space/tagp.h"

int ki_generic(int fd, int flag)
{
	int rc;
	tagp_interface_t tif;

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
An example of using in_data to pass in a structure:ki_write_t wif;
tagp_interface_t tif;

//fill out wif structure

/*
 * build interface structure
 */
tif.in_len = sizeof(ki_write_t);
tif.in_data = (caddr_t) & wif;
tif.out_len = 0;
tif.out_data = 0;
tif.out_rc = 0;

//make ioctl call

An example of using out_data to get back a structure:ki_read_t rif;
tagp_interface_t tif;

//fill out rif structure
rif.len = p_test->data[0];
rif.handle = open_handle;
rif.data = (caddr_t) p_test->data[1];

/*
 * build interface structure
 */
tif.in_len = sizeof(ki_read_t);
tif.in_data = (caddr_t) & rif;
tif.out_len = 0;
tif.out_data = 0;
tif.out_rc = 0;

//make ioctl call

#endif
