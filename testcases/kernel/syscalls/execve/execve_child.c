/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * test3.c
 *	dummy program which is used by execve02/4/5.c testcase
 */

#define TST_NO_DEFAULT_MAIN
#include <stdlib.h>
#include "tst_test.h"

int main(int argc, char *argv[])
{
	tst_reinit();

	/* For execve05 test, it should be returned here */
	if (argc > 1 && !strcmp(argv[1], "canary")) {
		tst_res(TPASS, "argv[1] is %s, expected 'canary'", argv[1]);
		return 0;
	}

	/* For execve02/4 test, it shouldn't be executed */
	tst_res(TFAIL, "%s shouldn't be executed", argv[0]);
	return 0;
}
