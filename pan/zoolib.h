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
 *
 */
/* $Id: zoolib.h,v 1.5 2006/06/27 09:37:34 vapier Exp $ */
#ifndef ZOOLIB_H
#define ZOOLIB_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/signal.h>

typedef FILE *zoo_t;
#define ZELEN 512
extern char zoo_error[ZELEN];
#define BUFLEN 81

int lock_file( FILE *fp, short ltype, char **errmsg );
/* FILE *open_file( char *file, char *mode, char **errmsg ); */

void wait_handler();

/*  char *zoo_active( void ); */
/* zoo_getname(): create a filename to use for the zoo
 * 	returns NULL on error */
char *zoo_getname(void);

/* zoo_open(): open a zoo file for use
 * 	returns NULL on error */
zoo_t zoo_open(char *zooname);

/* zoo_close(): close an open zoo file */
int zoo_close(zoo_t z);

/* zoo_mark_cmdline(): make an entry to the zoo
 *	returns 0 on success, -1 on error */
int zoo_mark_cmdline(zoo_t z, pid_t p, char *tag, char *cmdline);

/* zoo_mark_args(): make an entry to the zoo using argc argv
 *	returns 0 on success, -1 on error */
int zoo_mark_args(zoo_t z, pid_t p, char *tag, int ac, char **av);

/* zoo_clear(): mark a pid as completed
 *	returns 0 on success, -1 on error, 1 as warning */
int zoo_clear(zoo_t z, pid_t p);

/* zoo_getpid(): get the pid for a specified tag
 * 	returns pid_t on success and 0 on error */
pid_t zoo_getpid(zoo_t z, char *tag);


#endif /* ZOOLIB_H */
