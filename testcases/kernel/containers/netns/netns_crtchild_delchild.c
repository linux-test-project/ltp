/*************************************************************************
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
***************************************************************************/
/*========================================================================
* This testcase uses the libnetns.c from the lib to create network NS1.
* In libnetns.c it uses 2 scripts parentns.sh and childns.sh to create this.
*
* This testcase creates Network-NS NS1 and renames the net-dev of NS1 to veth4.
* Then it kills the Network-NS, and checks the contents of parent sysfs.
* The system should remain stable and also checks sysfs contents of parent NS.
* It returns PASS on Success else returns FAIL.
*
* Scripts used: netns_parentns.sh, netns_childns.sh , netns_delchild.sh
* netns_rename_net.sh
*
* Author: Veerendra C <vechandr@in.ibm.com>
*                      31/07/2008
* =========================================================================*/

#include "common.h"
#include "netns_helper.h"

#define IPROUTE_MIN_VER 80725

const char *TCID = "netns_crtchild_delchild";

static void setup(void)
{
	tst_require_root();
	check_iproute(IPROUTE_MIN_VER);
	check_netns();
}

int main(void)
{
	int status;
	setup();
	status = create_net_namespace("netns_delchild.sh", "netns_rename_net.sh");
	if (status == 0)
		tst_resm(TPASS, "create_net_namespace");
	else
		tst_resm(TFAIL, "create_net_namespace");
	tst_exit();
}
