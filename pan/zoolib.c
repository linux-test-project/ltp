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
/* $Id: zoolib.c,v 1.1 2000/09/14 21:54:44 nstraz Exp $ */
#include <stdlib.h> /* for getenv */
#include "zoolib.h"

#define A_BUF_SZ 100

static char Errmsg[500];

char *
zoo_active()
{
	char *fname = "active";
	static char *active = NULL;
	char *zoo;

	if( active == NULL ){
		zoo = getenv( "ZOO" );
		if( zoo != NULL ){
			active = (char*)malloc( strlen(zoo) + 1 +
					       strlen(fname) + 1 );
			sprintf( active, "%s/%s", zoo, fname );
		}
	}
	return active;
}


FILE *
open_file( char *file, char *mode, char **errmsg )
{
	FILE *fp;

	if( errmsg != NULL )
		*errmsg = Errmsg;

	if( strcmp( mode, "r+" ) == 0 ){
		/* make sure there's a file */
		if( (fp = fopen( file, "a" )) == NULL ){
			sprintf(Errmsg, "Unable to create file '%s'.  %s/%d  errno:%d  %s\n",
				file, __FILE__, __LINE__, errno, SYSERR );
			return(NULL);
		}
		else
			fclose( fp );
	}
	

	if( (fp = fopen( file, mode )) == NULL ){
		sprintf(Errmsg, "Unable to open(%s,%s).  %s/%d  errno:%d  %s\n",
			file, mode, __FILE__, __LINE__, errno, SYSERR );
		return(NULL);
	}
	return fp;
}

int
seek_file( FILE *fp, long int offset, int whence, char **errmsg )
{
	if( errmsg != NULL )
		*errmsg = Errmsg;

	if( (whence != SEEK_SET) && (whence != SEEK_END) ){
		sprintf(Errmsg, "whence is bad.  %s/%d\n", __FILE__, __LINE__ );
		return(-1);
	}

	if( fseek( fp, offset, whence ) != 0 ){
		sprintf(Errmsg,"fseek(%ld,%s) failed.  %s/%d  errno:%d  %s\n",
			offset,
			whence == SEEK_SET ? "SEEK_SET" : "SEEK_END",
			__FILE__, __LINE__,
			errno, SYSERR );
		return(-1);
	}
	return whence;
}


int
lock_file( FILE *fp, short ltype, char **errmsg )
{
	struct flock flk;
	int ret=1;

	if( errmsg != NULL )
		*errmsg = Errmsg;

	if( (ltype != F_WRLCK) && (ltype != F_UNLCK) ){
		sprintf(Errmsg, "bad ltype, %s/%d\n", __FILE__, __LINE__ );
		return(-1);
	}

	flk.l_whence = flk.l_start = flk.l_len = 0;
	flk.l_type = ltype;
/* XXX it's time to upgrade this code for sigaction */
	sighold(SIGINT);
	sighold(SIGTERM);
	sighold(SIGHUP);
	sighold(SIGUSR1);
	sighold(SIGUSR2);
	if( fcntl( fileno(fp), F_SETLKW, &flk ) == -1 ){
		sprintf(Errmsg, "fcntl(%s) failed. %s/%d  errno:%d  %s\n",
			ltype == F_WRLCK ? "F_WRLCK" : "F_UNLCK",
			__FILE__, __LINE__,
			errno, SYSERR );
		ret=-1;
	}
	sigrelse(SIGINT);
	sigrelse(SIGTERM);
	sigrelse(SIGHUP);
	sigrelse(SIGUSR1);
	sigrelse(SIGUSR2);
	return(ret);
}


int
write_active( FILE *fp, char *name, char **errmsg )
{
	pid_t pid = getpid();
	struct flock flk;
	char *emsg;

	if( fp == NULL )
		return(1);

	if( errmsg != NULL )
		*errmsg = Errmsg;

	if( lock_file( fp, F_WRLCK, &emsg ) == -1 )
		return(-1);
	if( seek_file( fp, 0, SEEK_END, &emsg ) == -1 )
		return(-1);

	/* Write pid first so update_active() can find it quickly.
	 * update_active() will probably be used far more often than any
	 * process that needs to search by name anyway.
	 */
	fprintf( fp, "%d,%s\n", pid, name );
	fflush( fp );

	if( lock_file( fp, F_UNLCK, &emsg ) == -1 )
		return(-1);
	return(1);
}

/*
 * Write a command to the active file, with arguments.
 * Uses a "first fit" algorithm to take the first available line
 * Inactive commandlines are assumed to start with a "#"
 */
int
write_active_args( FILE *fp, pid_t pid, char *name, int argc,
		  char **argv, char **errmsg )
{
	struct flock flk;
	char *args, *cat_args();
	char active[81];		/* max length.. */
	int len, l2, found;
	long int pos;
	char buf[A_BUF_SZ];
	char *emsg;

	if( fp == NULL )
		return(1);

	if( errmsg != NULL )
		*errmsg = Errmsg;

	if( (args = cat_args(argc, argv, &emsg)) == NULL )
		return(-1);
	len = sprintf(active, "%d,%s,", pid, name);
	strncat(active, args, 80 - len);
	len = strlen(active);
	free(args);

	if( lock_file( fp, F_WRLCK, &emsg ) == -1 )
		return(-1);
	if( seek_file( fp, 0, SEEK_SET, &emsg ) == -1 )
		return(-1);

	found=0;
	while(1) {
		pos = ftell( fp );
		if( fgets( buf, A_BUF_SZ - 1, fp ) == NULL )
			break;

		if( buf[0] == '#' ) {
		    /* "First Fit" */
		    if( (l2=strlen(buf)) > len ) { /* can rewrite this one */
			if( seek_file( fp, pos, SEEK_SET, &emsg ) == -1 )
				return(-1);
			fprintf( fp, "%s%*.*s\n", active, l2-len-1, l2-len-1,
				"|");
			found=1;
			break;
		    }
		}
	}/* while */

	if(!found) {
	    if( seek_file( fp, 0, SEEK_END, &emsg ) == -1 )
		return(-1);

	    /* Write pid first so update_active() can find it quickly.
	     * update_active() will probably be used far more often than any
	     * process that needs to search by name anyway.
	     */
	    fprintf( fp, "%s\n", active );
	}

	fflush( fp );
	if( lock_file( fp, F_UNLCK, &emsg ) == -1 )
		return(-1);

	return(1);
}


int
clear_active( FILE *fp, pid_t me, char **errmsg )
{
	char buf[A_BUF_SZ];
	int pid;
	long int pos;
	int found = 0;
	char *emsg;

	if( fp == NULL )
		return(1);

	if( lock_file( fp, F_WRLCK, &emsg ) == -1 )
		return(-1);
	if( seek_file( fp, 0, SEEK_SET, &emsg ) == -1 )
		return(-1);

	while(1){
		pos = ftell( fp );
		if( fgets( buf, A_BUF_SZ - 1, fp ) == NULL )
			break;

		if( buf[0] == '#' )
			continue;

		/* is pid me? */
		pid = atoi( buf );
		if( pid != me )
			continue;

		if( seek_file( fp, pos, SEEK_SET, &emsg ) == -1 )
			return(-1);
		fprintf( fp, "#" );
		found++;
		break;
	}/* while */

	fflush( fp );
	if( lock_file( fp, F_UNLCK, &emsg ) == -1 )
		return(-1);

	if( ! found ){
		sprintf(Errmsg, "clear_active() did not find pid(%d).  %s/%d\n",
			me, __FILE__, __LINE__ );
		return(0);
	}
	return(1);
}


void
wait_handler( int sig )
{
	static int lastsent = 0;

	if( sig == 0 ){
		lastsent = 0;
	}
	else {
		rec_signal = sig;
		if( sig == SIGUSR2 )
			return;
		if( lastsent == 0 )
			send_signal = sig;
		else if( lastsent == SIGUSR1 )
			send_signal = SIGINT;
		else if( lastsent == sig )
			send_signal = SIGTERM;
		else if( lastsent == SIGTERM )
			send_signal = SIGHUP;
		else if( lastsent == SIGHUP )
			send_signal = SIGKILL;
		lastsent = send_signal;
	}
}

char *
cat_args(int argc, char **argv, char **errmsg)
{
    int a, size;
    char *cmd;

    for(size=a=0;a<argc;a++) {
	size += strlen(argv[a]);
	size++;
    }

    if( (cmd = (char *)malloc(size)) == NULL ) {
	sprintf(Errmsg, "Malloc Error, %s/%d", __FILE__, __LINE__);
	return(NULL);
    }

    *cmd='\0';
    for(a=0;a<argc;a++) {
	if(a != 0)
	    strcat(cmd, " ");
	strcat(cmd, argv[a]);
    }

    return(cmd);
}
