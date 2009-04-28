/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2007, 2008           */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/
/*
 * File: exec_without_inh.c
 * Author: Serge Hallyn
 * Make sure that CAP_SYS_ADMIN is not in pI
 * drop CAP_SYS_ADMIN from bounding set
 * Then exec "check_pe 0"
 * check_pe will return PASS if it does not have CAP_SYS_ADMIN in pE.
 */

#include <errno.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <test.h>

#ifndef CAP_LAST_CAP
#warning out-of-date capability.h does not define CAP_LAST_CAP
#define CAP_LAST_CAP 28  /* be ultra-conservative */
#endif

#ifndef CAP_BSET_READ
#warning CAP_BSET_READ not defined
#define CAP_BSET_READ 23
#endif

#ifndef CAP_BSET_DROP
#warning CAP_BSET_DROP not defined
#define CAP_BSET_DROP 24
#endif

char *TCID = "exec_without_inh";
int TST_TOTAL=1;

int errno;

int main(int argc, char *argv[])
{
	int ret = 1;
	cap_flag_value_t f;
	cap_value_t v[1];
	cap_t cur;

	/* Make sure CAP_SYS_ADMIN is not in pI */
	cur = cap_get_proc();
	ret = cap_get_flag(cur, CAP_SYS_ADMIN, CAP_INHERITABLE, &f);
	if (f == CAP_SET) {
		v[0] = CAP_SYS_ADMIN;
		ret = cap_set_flag(cur, CAP_INHERITABLE, 1, v, CAP_CLEAR);
		if (!ret)
			ret = cap_set_proc(cur);
		if (ret) {
			tst_resm(TBROK, "Failed to drop cap_sys_admin from pI\n");
			tst_exit(1);
		}
	}
	cap_free(cur);

	/* drop the capability from bounding set */
	ret = prctl(CAP_BSET_DROP, CAP_SYS_ADMIN);
	if (ret) {
		tst_resm(TFAIL, "Failed to drop CAP_SYS_ADMIN from bounding set.\n");
		tst_resm(TINFO, "(ret=%d, errno %d)\n", ret, errno);
		tst_exit(1);
	}

	/* execute "check_pe 0" */
	execl("check_pe", "check_pe", "0", NULL);
	tst_resm(TBROK, "Failed to execute check_pe (errno %d)\n", errno);
	tst_exit(1);
}
