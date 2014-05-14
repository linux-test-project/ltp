/*
 * Copyright (C) 2012 Marios Makris <marios.makris@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Test program for the tst_tmpdir program in /lib
 *
 * This program creates and deletes a temporary file in order to test
 * the functionality of the tst_tmpdir functionality.
 * On successfull completion it prints the message:
 * "Test completed successfully!"
 */

#include <stdio.h>
#include <errno.h>

#include "test.h"

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX	MAXPATHLEN
#else
#define PATH_MAX	1024
#endif
#endif

char *TCID = "tst_tmpdir_test";
int TST_TOTAL = 1;

int main(void)
{
	char *tmp_dir;
	char *start_dir = getcwd(NULL, PATH_MAX);
	char *changed_dir;
	int fail_counter = 0;

	tst_tmpdir();

	tmp_dir = tst_get_tmpdir();
	changed_dir = getcwd(NULL, PATH_MAX);

	if (strcmp(tmp_dir, changed_dir) == 0 &&
	    strcmp(tmp_dir, start_dir) != 0) {
		printf("Temp directory successfully created and switched to\n");
	} else {
		printf("Temp directory is wrong!\n");
		fail_counter++;
	}

	tst_rmdir();

	if (chdir(tmp_dir) == -1 && errno == ENOENT) {
		printf("The temp directory was removed successfully\n");
	} else {
		printf("Failed to remove the temp directory!\n");
		fail_counter++;
	}

	if (fail_counter > 0)
		printf("Something failed please review!!\n");
	else
		printf("Test completed successfully!\n");

	return 0;
}
