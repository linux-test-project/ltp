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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: zoolib.h,v 1.1 2000/09/14 21:54:44 nstraz Exp $ */
#ifndef ZOOLIB_H
#define ZOOLIB_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/signal.h>

#ifdef NANNY
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN int rec_signal;	/* received signal */
EXTERN int send_signal;	/* signal to send */

extern int errno;
#ifndef linux
extern char *sys_errlist[];
#endif
#define SYSERR sys_errlist[errno]

int lock_file( FILE *fp, short ltype, char **errmsg );
FILE *open_file( char *file, char *mode, char **errmsg );

void wait_handler();

char *zoo_active( void );
int write_active( FILE *fp, char *name, char **errmsg );
int clear_active( FILE *fp, pid_t me, char **errmsg );
int write_active_args( FILE *fp, pid_t pid, char *name, int argc, char **argv, char **errmsg );
int seek_file( FILE *fp, long int offset, int whence, char **errmsg );
char *cat_args(int argc, char **argv, char **errmsg);

#endif /* ZOOLIB_H */
