/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
 * Created by: Sebastien Decugis

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *

 * This sample test aims to check the following assertion:
 * The function fails and return EPERM if caller has not the
 * privilege to perform the operation.

 * The steps are:
 * -> if this implementation does not support privileges, return PTS_UNSUPPORTED
 * -> Otherwise, use the implementation features to come to a situation where
 *      pthread_mutex_init should fail because of the privileges, and then check
 *      that the return code is EPERM.
 * -> return PTS_UNTESTED if the architecture is not present in the test.
 */


/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/utsname.h>
#include <string.h>

/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
#include "../../testfrmw/testfrmw.h"
#include "../../testfrmw/testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);
  *    where descr is a description of the error and ret is an int (error code for example)
  * FAILED(descr);
  *    where descr is a short text saying why the test has failed.
  * PASSED();
  *    No parameter.
  *
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  *
  * The other file defines the functions
  * void output_init()
  * void output(char * string, ...)
  *
  * Those may be used to output information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

#ifndef PTS_UNSUPPORTED
#define PTS_UNSUPPORTED 4
#endif
#ifndef PTS_UNTESTED
#define PTS_UNTESTED 5
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
int main(void)
{
	int ret;
	struct utsname un;

	output_init();
	ret = uname(&un);
	if (ret == -1) {
		UNRESOLVED(errno, "Unable to get Implementation name");
	}
#if VERBOSE > 0
	output("Implementation is: \n\t%s\n\t%s\n\t%s\n", un.sysname,
	       un.release, un.version);
#endif

	/* If we are running Linux */
	if (strcmp(un.sysname, "Linux") == 0) {
		/* Linux does not provide privilege access to pthread_mutex_init function */
		ret = PTS_UNSUPPORTED;
		output("Linux does not provide this feature\n");
		output_fini();
		return ret;
	}

	/* If we are running AIX */
	if (strcmp(un.sysname, "AIX") == 0) {
		;
	}
	/* If we are running Solaris */
	if (strcmp(un.sysname, "SunOS") == 0) {
		;
	}

	output("This implementation is not tested yet\n");
	output_fini();
	return PTS_UNTESTED;
}
