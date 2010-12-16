/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2001                 */
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
/* along with this program;  if not, write to the Free Software		          */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*									                                          */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        check_tray.c                                                  */
/*                                                                            */
/* Description: This program checks the status of the cdrom drive, it will    */
/*              return the status as to if the cdrom device is open or is     */
/*              ready for use.                                                */
/*                                                                            */
/* History:                                                                   */
/* Jan 8 2003 - Created - Manoj Iyer manjo@mail.utexas.edu                    */
/*            - Note: In the 2.4.19 kenrel ioctl() & this so this program     */
/*              '2' even when there is no cdrom in the drive. This might be   */
/*              a kbug. So value 1 is not used in the script to check if      */
/*              drive is empty.                                               */
/*                                                                            */
/******************************************************************************/

#include <sys/stat.h>
#include <fcntl.h>
#include <linux/cdrom.h>
#include <sys/ioctl.h>
#include <stdlib.h>

/******************************************************************************/
/*                                                                            */
/* Function:	main                                                          */
/*                                                                            */
/* Description: This function opens the cdrom device, and checks the status   */
/*              of the drive. Note drop the cdrom inside the cd drive for     */
/*              this program to work usefully.                                */
/*                                                                            */
/* Exit Vaules: 0 - No information.                                           */
/*              1 - No disk in the drive.                                     */
/*              2 - CD tray is open.                                          */
/*              3 - CD drive not ready.                                       */
/*              4 - CD disk in drive & drive closed.                          */
/*                                                                            */
/******************************************************************************/

int
main()
{
    int fdcdrom = -1;

	if ((fdcdrom = open("/dev/cdrom", O_RDONLY|O_NONBLOCK)) == -1)
		exit(-2);

	exit (ioctl(fdcdrom, CDROM_DRIVE_STATUS));
}