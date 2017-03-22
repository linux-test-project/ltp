/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
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

#ifndef LTP_AIODIO_COMMON_CHECKZERO
#define LTP_AIODIO_COMMON_CHECKZERO

static char *check_zero(char *buf, int size)
{
	char *p;

	p = buf;

	while (size > 0) {
		if (*buf != 0) {
			fprintf(stderr, "non zero buffer at buf[%u] => 0x%02x,%02x,%02x,%02x\n",
				(unsigned int)(buf - p),
				(unsigned int)buf[0],
				size > 1 ? (unsigned int)buf[1] : 0,
				size > 2 ? (unsigned int)buf[2] : 0,
				size > 3 ? (unsigned int)buf[3] : 0);
			return buf;
		}
		buf++;
		size--;
	}

	return NULL;
}

#endif /* LTP_AIODIO_COMMON_CHECKZERO */
