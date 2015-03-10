/*
 * Copyright (C) 2015 Cyril Hrubis chrubis@suse.cz
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

#include "test.h"

char *TCID = "execl01_child";

int main(int argc, char *argv[])
{
	if (argc != 2)
		tst_brkm(TFAIL, NULL, "argc is %d, expected 2", argc);

	if (strcmp(argv[1], "canary")) {
		tst_brkm(TFAIL, NULL, "argv[1] is %s, expected 'canary'",
		         argv[1]);
	}

	tst_resm(TPASS, "%s executed", argv[0]);
	tst_exit();
}
