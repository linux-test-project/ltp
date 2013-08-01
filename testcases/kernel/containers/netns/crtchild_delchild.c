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
* Scripts used: parentns.sh, childns.sh , delchild.sh rename_net.sh
*
* Author: Veerendra C <vechandr@in.ibm.com>
*                      31/07/2008
* =========================================================================*/

#include "libclone.h"

const char *TCID = "crtchild_delchild";

int main(void)
{
	int status;
	status = create_net_namespace("delchild.sh", "rename_net.sh");
	return status;
}
