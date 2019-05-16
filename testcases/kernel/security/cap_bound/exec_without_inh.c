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
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
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
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <linux/types.h>
#include <sys/capability.h>
#endif
#include <sys/prctl.h>
#include "test.h"

char *TCID = "exec_without_inh";
int TST_TOTAL = 1;

int main(int argc, char *argv[])
{
#if HAVE_SYS_CAPABILITY_H
#if HAVE_DECL_PR_CAPBSET_DROP
#ifdef HAVE_LIBCAP
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
			tst_brkm(TBROK,
				 NULL,
				 "Failed to drop cap_sys_admin from pI\n");
		}
	} else if (ret) {
		tst_brkm(TBROK | TERRNO, NULL, "Failed to add \
			CAP_SYS_ADMIN to pI");
	}
	cap_free(cur);

	/* drop the capability from bounding set */
	ret = prctl(PR_CAPBSET_DROP, CAP_SYS_ADMIN);
	if (ret) {
		tst_resm(TFAIL,
			 "Failed to drop CAP_SYS_ADMIN from bounding set.\n");
		tst_resm(TINFO, "(ret=%d, errno %d)\n", ret, errno);
		tst_exit();
	}

	/* execute "check_pe 0" */
	execl("check_pe", "check_pe", "0", NULL);
	tst_resm(TBROK, "Failed to execute check_pe (errno %d)\n", errno);
#else /* libcap */
	tst_resm(TCONF, "System doesn't have POSIX capabilities.");
#endif
#else /* HAVE_DECL_PR_CAPBSET_DROP */
	tst_resm(TCONF, "System doesn't have CAPBSET prctls");
#endif
#else /* capability_h */
	tst_resm(TCONF, "System doesn't have sys/capability.h.");
#endif
	tst_exit();
}
