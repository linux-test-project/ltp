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
/* $Id: bump.c,v 1.1 2000/09/14 21:54:44 nstraz Exp $ */
#include <stdio.h>
#include <errno.h>
#include <sys/signal.h>
#include <string.h>

#define NANNY /* not really */
#include "zoolib.h"

pid_t
read_active( FILE *fp, char *name );

static char *errmsg;

main( int argc, char **argv ){
	extern char *optarg;
	extern int optind;
	int c;
	int x;
	char *active = NULL;
	pid_t nanny;
	FILE *fp;
	int sig = SIGINT;

	while( (c = getopt(argc, argv, "a:s:12")) != -1 ){
		switch(c){
			case 'a':
				active = (char*)malloc(strlen(optarg)+1);
				strcpy( active, optarg );
				break;
			case 's':
				sig = atoi( optarg );
				break;
			case '1':
				sig = SIGUSR1;
				break;
			case '2':
				sig = SIGUSR2;
				break;
		}
	}

	if( active == NULL ){
		active = zoo_active();
		if( active == NULL ){
			fprintf(stderr, "bump: Must supply -a or set ZOO env variable\n");
			exit(1);
		}
	}
	if( optind == argc ){
		fprintf( stderr, "bump: Must supply names\n");
		exit(1);
	}

	/* need r+ here because we're using write-locks */
	if( (fp = open_file( active, "r+", &errmsg )) == NULL ){
		fprintf(stderr, "bump: %s\n", errmsg);
		exit(1);
	}
	while( optind < argc ){
		/*printf("argv[%d] = (%s)\n", optind, argv[optind] );*/
		nanny = read_active( fp, argv[optind] );
		if( nanny == -1 ){
			fprintf(stderr, "bump: Did not find name '%s'\n",
				argv[optind] );
		}
		else{
			if( kill( nanny, sig ) == -1 ){
				if( errno == ESRCH ){
					fprintf(stderr,"bump: Did not find nanny '%s', pid %d\n",
						argv[optind], nanny );
					if( clear_active( fp, nanny, &errmsg ) != 1 )
						fprintf(stderr,"bump: %s\n", errmsg);
				}
			}
		}
		++optind;
	}
	fclose( fp );

	exit(0);
}




pid_t
read_active( FILE *fp, char *name )
{
	char buf[1024];
	pid_t nanny = -1;
	char *n;
	int len;

	if( lock_file( fp, F_WRLCK, &errmsg ) == -1 ){
		fprintf(stderr, "bump: %s\n", errmsg );
		return(-1);
	}
	if( seek_file( fp, 0, SEEK_SET, &errmsg ) == -1 ){
		fprintf(stderr, "bump: %s\n", errmsg );
		return(-1);
	}

	len = strlen( name );
	while(1){
		if( fgets( buf, 1023, fp ) == NULL ){
			/*printf("end of file\n");*/
			break;
		}

		if( buf[0] == '#' ){
			/*printf("skip line (%s)\n", buf );*/
			continue;
		}

		/* get name */
		if( (n = strchr( buf, ',' )) == NULL ){
			/*printf("no comma (%s)\n", buf );*/
			continue;
		}

		if( strncmp( n + 1, name, len ) != 0 ){
			/*printf("not matching (%s) and (%s)\n",
			       n+1, name );*/
			continue;
		}

		nanny = atoi( buf );
		break;
	}/* while */

	if( lock_file( fp, F_UNLCK, &errmsg ) == -1 ){
		fprintf(stderr, "bump: %s\n", errmsg );
		return(-1);
	}
	return nanny;
}

