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
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#endif
#include <sys/prctl.h>
#include <test.h>

char *TCID = "exec_without_inh";
int TST_TOTAL=1;

int errno;

int main(int argc, char *argv[])
{
#if HAVE_SYS_CAPABILITY_H
	int ret = 1;
	cap_flag_value_t f;
	cap_value_t v[1];
	cap_t cur;

	/* Make sure CAP_SYS_ADMIN is not in pI */
#if HAVE_DECL_CAP_GET_FLAG
#if HAVE_DECL_CAP_GET_PROC
	cur = cap_get_proc();
	ret = cap_get_flag(cur, CAP_SYS_ADMIN, CAP_INHERITABLE, &f);
#else
	ret = -1;
	errno = ENOSYS;
#endif
#else
	ret = -1;
	errno = ENOSYS;
#endif
	if (f == CAP_SET) {
		v[0] = CAP_SYS_ADMIN;
#if HAVE_DECL_CAP_SET_FLAG
		ret = cap_set_flag(cur, CAP_INHERITABLE, 1, v, CAP_CLEAR);
#else
		ret = -1;
		errno = ENOSYS;
#endif
#if HAVE_DECL_CAP_SET_PROC
		if (!ret)
			ret = cap_set_proc(cur);
#else
		ret = -1;
		errno = ENOSYS;
#endif
		if (ret) {
			tst_resm(TBROK, "Failed to drop cap_sys_admin from pI\n");
			tst_exit();
		}
	}
#if HAVE_DECL_CAP_FREE
	cap_free(cur);
#endif

	/* drop the capability from bounding set */
#if HAVE_DECL_CAP_BSET_DROP
	ret = prctl(CAP_BSET_DROP, CAP_SYS_ADMIN);
#else
	errno = ENOSYS;
	ret = -1;
#endif
	if (ret) {
		tst_resm(TFAIL, "Failed to drop CAP_SYS_ADMIN from bounding set.\n");
		tst_resm(TINFO, "(ret=%d, errno %d)\n", ret, errno);
		tst_exit();
	}

	/* execute "check_pe 0" */
	execl("check_pe", "check_pe", "0", NULL);
	tst_resm(TBROK, "Failed to execute check_pe (errno %d)\n", errno);
#else
	tst_resm(TCONF, "System doesn't have POSIX capabilities.");
#endif
	tst_exit();
}
