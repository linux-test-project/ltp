/*
 * Written by Serge E. Hallyn <serue@us.ibm.com>
 * Copyright (c) International Business Machines  Corp., 2005
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <dlfcn.h>

int test(void *handle)
{
	int (*square)(int);
	const char *error;

	printf("testing\n");
	square = dlsym(handle, "square");
	if ((error = dlerror()) != NULL)  {
		fputs(error, stderr);
		exit(-1);
	}

	printf ("%d\n", (*square)(3));
	printf("done testing\n");
}

int main(int argc, char **argv) {
	void *handle;
	handle = dlopen ("./shared.so", RTLD_LAZY);
	if (!handle) {
		fputs (dlerror(), stderr);
		sleep(4);
		exit(1);
	}

	test(handle);
	if (argc > 1) {
		sleep(4);
		test(handle);
	}
	dlclose(handle);
	exit(0);
}
