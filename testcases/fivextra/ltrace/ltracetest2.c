/*
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003                                               ##
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
# File:         ltracetest2.c
#
# Description:  This program tests basic functionality of ltrace program
#
# Author:       CSDL,  James He <hejianj@cn.ibm.com>
#
# History:      16 April 2004 - created - James He
*/

#include <stdio.h>
#include <stdlib.h>
main(){
	int constrpos;
	char constr[]="abcdefghijklmnopqrstuvwxyz";
	for(constrpos=0;constrpos<sizeof(constr);constrpos++)
		printf("%c",toupper(constr[constrpos]));

	printf("\n");
	exit(0);

}
