/*
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2004                                               ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File:         sudotest.c
#
# Description:  This program tests basic functionality of sudo program
#
# Author:       CSDL,  James He <hejianj@cn.ibm.com>
#
# History:      13 May 2004 - created - James He
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv)
{

    if (geteuid() != 0) {
	fprintf(stderr, "Sorry, %s must be setuid root.\n", argv[0]);
	exit(1);
    }
    else {
    	fprintf(stdout, "Ok, %s have been setuid root.\n", argv[0]);
	exit(0);
    }
}
