/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2009 FUJITSU LIMITED                                         */
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
/* Author: Miao Xie <miaox@cn.fujitsu.com>                                    */
/* Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>                     */
/*                                                                            */
/******************************************************************************/
/*++ simple_echo.c
 *
 * DESCRIPTION:
 *	The command "echo" can't return the errno. So we write this program to
 * instead of "echo".
 *
 * Author:
 * -------
 * 2008/04/17 created by Miao Xie@FNST
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int fd = 1;

	if (argc != 2 && argc != 3) {
		fprintf(stderr, "usage: %s STRING [ostream]\n",
			basename(argv[0]));
		exit(1);
	}

	if (argc == 3)
		if ((fd = open(argv[2], O_RDWR | O_SYNC)) == -1)
			err(errno, "%s", argv[2]);

	if (write(fd, argv[1], strlen(argv[1])) == -1)
		err(errno, "write error");

	if (fd != 1)
		if (close(fd) == -1)
			err(errno, "close error");
	return 0;
}
