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
* $Id: sfunc.h,v 1.5 2008/02/14 08:22:23 subrata_modak Exp $
* $Log: sfunc.h,v $
* Revision 1.5  2008/02/14 08:22:23  subrata_modak
* Disktest application update to version 1.4.2, by, Brent Yardley <yardleyb@us.ibm.com>
*
* Revision 1.13  2005/10/12 23:13:35  yardleyb
* Updates to code to support new function in disktest version 1.3.x.
* Actual changes are recorded in the README
*
* Revision 1.12  2005/05/03 16:24:38  yardleyb
* Added needed code changes to support windows
*
* Revision 1.11  2005/01/08 21:18:34  yardleyb
* Update performance output and usage.  Fixed pass count check
*
* Revision 1.10  2004/11/02 20:47:13  yardleyb
* Added -F functions.
* lots of minor fixes. see README
*
* Revision 1.9  2002/03/30 01:32:14  yardleyb
* Major Changes:
*
* Added Dumping routines for
* data miscompares,
*
* Updated performance output
* based on command line.  Gave
* one decimal in MB/s output.
*
* Rewrote -pL IO routine to show
* correct stats.  Now show pass count
* when using -C.
*
* Minor Changes:
*
* Code cleanup to remove the plethera
* if #ifdef for windows/unix functional
* differences.
*
* Revision 1.8  2002/02/28 02:04:32  yardleyb
* Moved FileSeek64 to IO
* source files.
*
* Revision 1.7  2002/02/19 02:46:37  yardleyb
* Added changes to compile for AIX.
* Update getvsiz so it returns a -1
* if the ioctl fails and we handle
* that fact correctly.  Added check
* to force vsiz to always be greater
* then stop_lba.
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

#ifndef _SFUNC_H
#define _SFUNC_H 1

#include <stdarg.h>

#include "main.h"
#include "defs.h"

typedef enum lvl {
	START, END, STAT, INFO, DBUG, WARN, ERR
} lvl_t;

typedef struct fmt_time {
	time_t days;
	time_t hours;
	time_t minutes;
	time_t seconds;
} fmt_time_t;

OFF_T my_strtofft(const char *pStr);
int pMsg(lvl_t level, const child_args_t *, char *Msg,...);
void fill_buffer(void *, size_t, void *, size_t, const unsigned int);
void mark_buffer(void *, const size_t, void *, const child_args_t *, const test_env_t *);
void normalize_percs(child_args_t *);
#ifndef WINDOWS
void Sleep(unsigned int);
char *strupr(char *);
char *strlwr(char *);
#endif
OFF_T get_vsiz(const char *);
OFF_T get_file_size(char *);
OFF_T Rand64(void);
fmt_time_t format_time(time_t);

#endif /* _SFUNC_H */

