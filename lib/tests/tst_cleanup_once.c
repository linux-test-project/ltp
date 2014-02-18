/*
 * Copyright (C) 2014 Linux Test Project
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

#include <pthread.h>

#include "test.h"

char *TCID = "tst_cleanup_once";
int TST_TOTAL = 1;

/*
 * This is called exactly once
 */
static void do_cleanup(void)
{
	printf("Cleanup\n");
}

TST_DECLARE_ONCE_FN(cleanup, do_cleanup);

int main(void)
{
	cleanup();
	cleanup();

	return 0;
}
