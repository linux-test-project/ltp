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
#ifndef _OPEN_FLAGS_H_
#define _OPEN_FLAGS_H_

/***********************************************************************
 * This function attempts to convert open flag bits into human readable
 * symbols (i.e. O_TRUNC).  If there are more than one symbol,
 * the <sep> string will be placed as a separator between symbols.
 * Commonly used separators would be a comma "," or pipe "|".
 * If <mode> is one and not all <openflags> bits can be converted to
 * symbols, the "UNKNOWN" symbol will be added to return string.
 *
 * Return Value
 * openflags2symbols will return the indentified symbols.
 * If no symbols are recognized the return value will be a empty
 * string or the "UNKNOWN" symbol.
 *
 * Limitations
 * Currently (05/96) all known symbols are coded into openflags2symbols.
 * If new open flags are added this code will have to updated
 * to know about them or they will not be recognized.
 *
 * The Open_symbols must be large enough to hold all possible symbols
 * for a given system.
 *
 ***********************************************************************/
char *openflags2symbols( int, char *, int );

/***********************************************************************
 * This function will take a string of comma separated open flags symbols
 * and translate them into an open flag bitmask.
 * If any symbol is not valid, -1 is returned.  On this error condition
 * the badname pointer is updated if not NULL.  badname will point
 * to the beginning location of where the invalid symbol was found.
 * string will be returned unchanged.
 *
 * A signal received while parsing string could cause the string to
 * contain a NULL character in the middle of it.
 *
 ***********************************************************************/
int parse_open_flags( char *, char ** );

#endif
