/************************************************************************
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
***************************************************************************/
#include <stdio.h>
#include "test.h"

int get_supp_sched_mc(void) {
	if (tst_kvercmp(2,6,29) < 0)
		return 1;
	else
		return 2;
}

int get_supp_sched_smt(void) {
	if (tst_kvercmp(2,6,29) < 0)
		return 1;
	else
		return 2;
}

int main(int argc, char **argv)
{
	char *param;
	if (argc == 0)
		return 1;
	else
	{
		param = argv[1];
		if (strcmp(param, "sched_mc")==0)
			return (get_supp_sched_mc());
		if (strcmp(param, "sched_smt")==0)
                        return (get_supp_sched_smt());
	}
}
