/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/******************************************************************************
 *  writen.c
 ******************************************************************************/

#include <unistd.h>

/* Write "n" bytes to a descriptor
   Use instead of write() when fd is a stream socket. */

int writen(fd, ptr, nbytes)
register int fd;
register char *ptr;
register int nbytes;
{
    int nleft, nwritten;

    nleft = nbytes;
    while (nleft > 0)
	{
	nwritten = write(fd, ptr, nleft);
	if (nwritten <= 0)
	    return(nwritten);     /* error */
	nleft -= nwritten;
	ptr += nwritten;
	}
    return(nbytes - nleft);
}
