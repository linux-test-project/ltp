/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2001                 */
/*  Jan 8 2003 - Created - Manoj Iyer manjo@mail.utexas.edu                   */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but        */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY */
/* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   */
/* for more details.                                                          */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software Foundation,   */
/* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           */
/******************************************************************************/

/*
 *
 * Description: This program checks the status of the cdrom drive, it will
 *              return the status as to if the cdrom device is open or is
 *              ready for use.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <linux/cdrom.h>
#include <sys/ioctl.h>
#include <stdlib.h>

/*
 * Exit Vaules:
 * 0 - No information.
 * 1 - No disk in the drive.
 * 2 - CD tray is open.
 * 3 - CD drive not ready.
 * 4 - CD disk in drive & drive closed.
 */
int main(int argc, char *argv[])
{
	int fd;

	if (argc != 2)
		exit(-1);

	if ((fd = open(argv[1], O_RDONLY | O_NONBLOCK)) == -1)
		exit(-2);

	exit(ioctl(fd, CDROM_DRIVE_STATUS));
}
