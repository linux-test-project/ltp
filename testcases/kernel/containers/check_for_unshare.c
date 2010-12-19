/*
* Copyright (c) International Business Machines Corp., 2008
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
**************************************************************************/
/*
* Description:
* This program verifies the kernel version to be no later than 2.6.16
* And checks if the unshare() system call is defined using dlsym(),
* in the Dynamically Linked Libraries.
*
* Date : 26-11-2008
* Author : Veerendra C <vechandr@in.ibm.com>
*/

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"

int main(int argc, char **argv)
{
	void *handle;
	void *ret;
	char *error;
	if (tst_kvercmp(2, 6, 16) < 0)
		return 1;

	handle = dlopen(NULL, RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}

	dlerror();    /* Clear any existing error */
	ret = dlsym(handle, "unshare");
	if ((error = dlerror()) != NULL)  {
		fprintf(stderr, "Error: %s\n", error);
		exit(1);
	}

	dlclose(handle);
	return 0;
}