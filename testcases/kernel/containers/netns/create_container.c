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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
***************************************************************************/
/* ============================================================================
* This testcase creates the child container to test the tcp command testcases
* inside the child namespace. The following program uses the library libclone
* api to create the Network namespace container and may be removed to use the
* containers management tools in future.
*
* The scripts runallnetworktests_parent.sh, runallnetworktests_child.sh, are
* passed as the parameters which will be running in the parent and child
* namespace respectively.
*
* Author: Sudhir Kumar <skumar@linux.vnet.ibm.com>   26/08/2008
* ============================================================================*/

#include "libclone.h"

int main()
{
	int status;
	/*
	 * The argument files contain the code to be run in the parent and
	 * child network namespace container respectively
	 */
	status = create_net_namespace("runallnetworktests_parent.sh",
						"runallnetworktests_child.sh");
	printf("Execution of all the network testcases under network"
			" namespace done. return value is %d\n", status);
	return status;
}