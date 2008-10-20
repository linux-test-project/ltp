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
* $Id: dump.h,v 1.5 2008/10/20 06:30:33 subrata_modak Exp $
*
*/
#ifndef _DUMP_H
#define _DUMP_H 1

#include "main.h"

#define FMT_STR 1
#define FMT_RAW 2

int dump_data(FILE *, const char *, const size_t, const size_t, const size_t, const int);
int do_dump(child_args_t *);

#endif /* _DUMP_H */

