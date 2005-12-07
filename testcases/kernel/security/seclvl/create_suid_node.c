/*######################################################################
##   Copyright (c) International Business Machines  Corp., 2003
##
##   This program is free software;  you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation; either version 2 of the License, or
##   (at your option) any later version.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY;  without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
##   the GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public License
##   along with this program;  if not, write to the Free Software
##   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##
##  FILE   : verify_seclvl.sh
##
##  PURPOSE: To test the compliance of the seclvl module.  This will
##           exit with an error code at the first sign of trouble.
##
##  HISTORY:
##    10/03 Originated by Michael A. Halcrow <mhalcrow@us.ibm.com>
##    02/05 Updated by Mike Halcrow to verify suid and guid permissions
##
######################################################################*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
	int fd;
	fd = mknod( "suid_node", S_IFREG | 04777, (dev_t)NULL );
	if (fd == -1) {
		printf( "Error creating suid node\n" );
		return 1;
	}
	printf( "Success creating suid node\n" );
	close( fd );
	return 0;
}
