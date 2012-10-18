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
* This testcase uses the libnetns.c from the library to create network NS.
* In libnetns.c it uses 2 scripts parentns.sh and childns.sh to create this.
* After creating the NS, this program verifies that the network is reachable
* from parent-NS to child-NS and vice-versa.
*
* Scripts Used: parentns.sh, childns.sh, par_ftp.sh , ch_ftp.sh container_ftp.pl
*
* Author: Veerendra C <vechandr@in.ibm.com>
*                      31/07/2008
* =============================================================================*/

#include "libclone.h"

int main()
{
    int status;
    status = create_net_namespace("par_ftp.sh", "ch_ftp.sh");
    return status;
}
