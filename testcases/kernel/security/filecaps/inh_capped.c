/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2008                 */
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
 * File: inh_capped
 * Author: Serge Hallyn
 * Purpose: test that CAP_SETPCAP is needed to add bits to pI
 * Uses no command line arguments.
 */

#include <stdio.h>
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <linux/types.h>
#include <sys/capability.h>
#endif
#include "test.h"

char *TCID = "filecaps";
int TST_TOTAL = 1;

#ifdef HAVE_LIBCAP
void debug_print_caps(char *when)
{
	char buf[2000];
	tst_resm(TINFO, "%s", when);
	snprintf(buf, 2000, "%s", cap_to_text(cap_get_proc(), NULL));
	tst_resm(TINFO, "%s", buf);
}

int set_caps_from_text(char *capstr)
{
	cap_t caps = cap_from_text(capstr);
	int ret;

	if (!caps) {
		tst_resm(TFAIL, "Bad capability name: %s\n", capstr);
		return 1;
	}
	ret = cap_set_proc(caps);
	cap_free(caps);
	return ret;
}
#endif

int main(void)
{
#ifdef HAVE_LIBCAP
	int ret;

	debug_print_caps("start");
	ret = set_caps_from_text("all=eip");
	debug_print_caps("after raising all caps");
	if (ret) {
		tst_brkm(TFAIL, NULL, "failed to raise all caps");
	}

	ret = set_caps_from_text("all=iep cap_sys_admin-iep");
	debug_print_caps("after first drop cap_sys_admin");
	if (ret) {
		tst_brkm(TFAIL, NULL, "failed to drop capsysadmin from pI");
	}

	/* we can't regain cap_sys_admin in pE or pP, only pI */
	ret = set_caps_from_text("all=eip cap_sys_admin-ep+i");
	debug_print_caps("after first raise cap_sys_admin");
	if (ret) {
		tst_brkm(TFAIL, NULL, "failed to raise capsysadmin in pI");
	}

	ret = set_caps_from_text("all=ip cap_setpcap-e+ip cap_sys_admin+i-ep");
	debug_print_caps("after drop cappset");
	if (ret) {
		tst_brkm(TFAIL, NULL, "failed to drop cappset from pE");
	}

	ret = set_caps_from_text("all=iep cap_sys_admin-iep cap_setpcap-e+ip");
	debug_print_caps("after second drop cap_sys_admin");
	if (ret) {
		tst_brkm(TFAIL, NULL, "failed to drop capsysadmin from pI "
			 "after dropping cappset from pE");
	}

	ret = set_caps_from_text("all=iep cap_sys_admin-ep+i cap_setpcap-e+ip");
	debug_print_caps("final");
	if (ret) {
		tst_resm(TPASS, "pI is properly capped\n");
		tst_exit();
	}

	tst_resm(TFAIL, "succeeded raising capsysadmin in pI "
		 "without having setpcap");
#else
	tst_resm(TCONF, "System doesn't have POSIX capabilities support.");
#endif
	tst_exit();
}
