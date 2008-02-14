/*
* $Id: timer.h,v 1.2 2008/02/14 08:22:24 subrata_modak Exp $
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
* $Id: timer.h,v 1.2 2008/02/14 08:22:24 subrata_modak Exp $
*
*/

#ifndef _TIMER_H_ /* _TIMER_H */
#define _TIMER_H_

void setStartTime(void);
void setEndTime(void);
unsigned long getTimeDiff(void);

#ifdef WINDOWS
DWORD WINAPI ChildTimer(test_ll_t *);
#else
void *ChildTimer(void *);
#endif


#endif /* _TIMER_H */
