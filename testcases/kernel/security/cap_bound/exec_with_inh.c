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
 * File: exec_with_inh.c
 * Author: Serge Hallyn
 * Make sure that CAP_SYS_ADMIN is in pI
 * drop CAP_SYS_ADMIN from bounding set
 * Then exec "check_pe 1"
 * check_pe will return PASS if it has CAP_SYS_ADMIN in pE.
 */

#include <errno.h>
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#endif
#include <sys/prctl.h>
#include <test.h>

char *TCID = "exec_with_inh";
int TST_TOTAL=1;

int errno;

int main(int argc, char *argv[])
{
#if HAVE_SYS_CAPABILITY_H
#ifdef HAVE_LIBCAP
	int ret = 1;
	cap_flag_value_t f;
	cap_t cur = 0;

	/* Make sure CAP_SYS_ADMIN is in pI */
	cur = cap_from_text("all=eip");
	if (!cur) {
		tst_resm(TBROK, "Failed to create cap_sys_admin+i cap_t (errno %d)\n", errno);
		tst_exit();
	}
	ret = cap_set_proc(cur);
	if (ret) {
		tst_resm(TBROK, "Failed to cap_set_proc with cap_sys_admin+i (ret %d errno %d)\n",
			ret, errno);
		tst_exit();
	}
	cap_free(cur);
	cur = cap_get_proc();
	ret = cap_get_flag(cur, CAP_SYS_ADMIN, CAP_INHERITABLE, &f);
	if (ret || f != CAP_SET) {
		tst_resm(TBROK, "Failed to add CAP_SYS_ADMIN to pI\n");
		tst_exit();
	}
	cap_free(cur);

	/* drop the capability from bounding set */
	ret = prctl(PR_CAPBSET_DROP, CAP_SYS_ADMIN);
	if (ret) {
		tst_resm(TFAIL, "Failed to drop CAP_SYS_ADMIN from bounding set.\n");
		tst_resm(TINFO, "(ret=%d, errno %d)\n", ret, errno);
		tst_exit();
	}

	/* execute "check_pe 1" */
	execl("check_pe", "check_pe", "1", NULL);
	tst_resm(TBROK, "Failed to execute check_pe (errno %d)\n", errno);
#else /* HAVE_LIBCAP */
	tst_resm(TCONF, "System doesn't have POSIX capabilities.");
#endif
#else /* HAVE_SYS_CAPABILITY_H */
	tst_resm(TCONF, "System doesn't have sys/capability.h.");
#endif
	tst_exit();
}
