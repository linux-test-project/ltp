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
/* ============================================================================
* This testcase uses the libnetns.c from the library to create network NS.
* In libnetns.c it uses 2 scripts parentns.sh and childns.sh to create this.
* After creating the NS, this program verifies that the network is reachable
* from parent-NS to child-NS and vice-versa.
*
* Scripts Used: netns_parentns.sh, netns_childns.sh, netns_parent.sh, netns_child.sh.
*
* Author: Veerendra C <vechandr@in.ibm.com>
*                      31/07/2008
* =============================================================================*/

#include "common.h"
#include "netns_helper.h"

#define IPROUTE_MIN_VER 80725

const char *TCID = "netns_crtchild";

static void setup(void)
{
	tst_require_root(NULL);
	check_iproute(IPROUTE_MIN_VER);
	check_netns();
}

int main(void)
{
	int status;

	setup();
	status = create_net_namespace("netns_parent.sh", "netns_child.sh");
	if (status == 0)
		tst_resm(TPASS, "create_net_namespace");
	else
		tst_resm(TFAIL, "create_net_namespace");
	tst_exit();
}
