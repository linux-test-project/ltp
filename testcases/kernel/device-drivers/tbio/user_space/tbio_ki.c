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
 * This file will include user space functions that will drive
 * the kernel module to test various functions and kernel
 * calls. Each function will need to setup the tif structure
 * so that the in parameters and out parameters are correctly
 * initialized
 *
 * use bif structure for passing params between user
 * space and kernel space, in some tests it is really not
 * needed, and if nothing is needed to pass in utilize
 * the ki_generic function below. the tif structure makes
 * it easy to maintain all the tests if they have the same
 * process in kernel space to read in params in the kernel
 * module no matter what the test is
 *
  * module: tbio
 *  Copyright (c) International Business Machines  Corp., 2003
 *
 *  This program is free software;  you can redistribute it and/or modify
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
 *  FILE        : tbio_ki.c
 *  USAGE       : kernel_space:./load_tbio.sh
 *                user_space  :./test_bio
 *
 *  DESCRIPTION : The module will test block i/o layer for kernel 2.5
 *  REQUIREMENTS:
 *                1) glibc 2.1.91 or above.
 *
 *  HISTORY     :
 *      11/19/2003 Kai Zhao (ltcd3@cn.ibm.com)
 *
 *  CODE COVERAGE: 74.9% - fs/bio.c (Total Coverage)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "../kernel_space/tbio.h"

int ki_generic(int fd, int flag)
{
	int                     rc;
	tbio_interface_t        bif;

	rc = ioctl(fd, flag, &bif);
	if (rc) {
		printf("Ioctl error\n");
		return rc;
	}

	return rc;
}

#if 0

#endif