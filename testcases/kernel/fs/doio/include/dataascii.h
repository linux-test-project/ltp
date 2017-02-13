/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */
#ifndef _DATAASCII_H_
#define _DATAASCII_H_

/***********************************************************************
 * int dataasciigen(listofchars, buffer, size, offset)
 *
 * This function fills buffer with ascii characters.
 * The ascii characters are obtained from listofchars or the CHARS array
 *  if listofchars is NULL.
 * Each char is selected by an index.  The index is the remainder
 *  of count divided by the array size.
 * This method allows more than one process to write to a location
 *  in a file without corrupting it for another process' point of view.
 *
 * The return value will be the number of character written in buffer
 *  (size).
 *
 ***********************************************************************/
int dataasciigen(char *, char *, int, int);

/***********************************************************************
 * int dataasciichk(listofchars, buffer, size, count, errmsg)
 *
 * This function checks the contents of a buffer produced by
 *  dataasciigen.
 *
 * return values:
 *	>= 0 : error at character count
 *	< 0  : no error
 ***********************************************************************/

int dataasciichk(char *, char *, int, int, char**);

#endif
