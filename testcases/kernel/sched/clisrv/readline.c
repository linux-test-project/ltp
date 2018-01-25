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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*****************************************************************************
 *  readline.c
 ******************************************************************************/

#include <unistd.h>

/* Read line from a descriptor, byte at a time, return number of
   characters up to the null */

int readline(int fd, char *ptr, int maxlen)
{
	int n, rc;
	char c;
	/*
	   printf("readline: fd = %d\n", fd);
	 */
	for (n = 1; n < maxlen; n++) {
		if ((rc = read(fd, &c, 1)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			if (n == 1)
				return 0;	/* EOF no data read */
			else
				break;	/* EOF, some data read */
		} else
			return (-1);	/* error */
	}
	*ptr = 0;
	return (n);
}
