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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/

/*
 * File: unhex.c
 * Author: Serge Hallyn
 * Purpose: Read a 40 char hex value from stdin, output 20 char byte
 * value on stdout.
 */

#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	char in[41], out[20];
	unsigned int v;
	int i, ret;

	ret = read(STDIN_FILENO, in, 40);
	if (ret != 40)
		return 1;
	in[40] = '\0';
	for (i = 0; i < 20; i++) {
		sscanf(&in[2*i], "%02x", &v);
		out[i] = v;
	}
	write(STDOUT_FILENO, out, 20);
	return 0;
}