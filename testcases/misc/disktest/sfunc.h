/*
* Disktest
* Copyright (c) International Business Machines Corp., 2001
*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Please send e-mail to yardleyb@us.ibm.com if you have
*  questions or comments.
*
*  Project Website:  TBD
*
*
* $Id: sfunc.h,v 1.1 2002/02/19 21:29:26 robbiew Exp $
* $Log: sfunc.h,v $
* Revision 1.1  2002/02/19 21:29:26  robbiew
* Added disktest.
*
* Revision 1.6  2001/12/04 18:51:06  yardleyb
* Checkin of new source files and removal
* of outdated source
*
* Revision 1.4  2001/10/10 00:17:14  yardleyb
* Added Copyright and GPL license text.
* Miner bug fixes throughout text.
*
* Revision 1.3  2001/09/22 03:44:25  yardleyb
* Added level code pMsg.
*
* Revision 1.2  2001/09/06 18:23:30  yardleyb
* Added duty cycle -D.  Updated usage. Added
* make option to create .tar.gz of all files
*
* Revision 1.1  2001/09/05 22:44:42  yardleyb
* Split out some of the special functions.
* added O_DIRECT -Id.  Updated usage.  Lots
* of clean up to functions.  Added header info
* to pMsg.
*
*
*/

#include <stdarg.h>

#include "main.h"
#include "defs.h"

#define MARK_FIRST	1
#define MARK_LAST	2
#define MARK_ALL	3

typedef enum lvl {
	START, END, STAT, INFO, DEBUG, WARN, ERR
} level_t;

OFF_T my_strtofft(const char *pStr);
int pMsg(level_t level, char *Msg,...);
void fill_buffer(void *, size_t, void *, size_t, const unsigned int);
void mark_buffer(void *, const size_t, void *, const OFF_T, const unsigned short);
void normalize_percs(child_args_t *args);
#ifdef WIN32
OFF_T FileSeek64(HANDLE hf, OFF_T distance, DWORD MoveMethod);
#else
void Sleep(unsigned int);
#endif
OFF_T get_vsiz(char *device);
OFF_T Rand64(void);
char *strupr(char *String);
char *strlwr(char *String);
