/*
 *   Copyright (c) Cyril Hrubis chrubis@suse.cz 2009
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

/*
 * This small library was created in order to put all code that's duplicated in
 * ftestXX.c files here.
 */

#ifndef __LIBFTEST_H__
#define __LIBFTEST_H__

struct iovec;

/*
 * Dump content of iov structure.
 */
void ft_dumpiov(struct iovec *iov);

/*
 * Dump bits string.
 */
void ft_dumpbits(void *bits, size_t size);

/*
 * Do logical or of hold and bits (of size)
 * fields and store result into hold field.
 */
void ft_orbits(char *hold, char *bits, int size);

/*
 * Dumps buffer in hexadecimal format.
 */
void ft_dumpbuf(char *buf, int csize);

/*
 * Creates filename from path and numbers.
 */
void ft_mkname(char *name, char *dirname, int me, int idx);

#endif /* __LIBFTEST_H__ */
