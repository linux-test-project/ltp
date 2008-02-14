/*
* Disktest
* Copyright (c) International Business Machines Corp., 2005
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
* $Id: signals.h,v 1.1 2008/02/14 08:22:24 subrata_modak Exp $
*/

#ifndef SIGNALS_H
#define SIGNALS_H 1

#define SIGNAL_NONE 0x0000
#define SIGNAL_STOP 0x0001
#define SIGNAL_STAT 0x0002

#ifdef WINDOWS
#define SIGQUIT	0x03	/* Definition from POSIX */
#define SIGHUP	0x01	/* Definition from POSIX */
#define SIGUSR1	0x10	/* Definition from POSIX */
#endif

void setup_sig_mask( void );
void clear_stat_signal( void );

#endif /* SIGNALS_H */
