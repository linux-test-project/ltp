/* $OpenLDAP: pkg/ldap/tests/progs/slapd-tester.c,v 1.9.6.4 2002/01/04 20:38:37 kurt Exp $ */
/*
 * Copyright 1998-2002 The OpenLDAP Foundation, All Rights Reserved.
 * COPYING RESTRICTIONS APPLY, see COPYRIGHT file
 */
/* #include "portable.h" */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/param.h>
#include <asm/socket.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include "ldap_defaults.h"

#define SEARCHCMD		"slapd-search"
#define READCMD			"slapd-read"
#define ADDCMD			"slapd-addel"
#define MAXARGS      	100
#define MAXREQS			20
#define LOOPS			"100"

#define TSEARCHFILE		"do_search.0"
#define TREADFILE		"do_read.0"
#define TADDFILE		"do_add."

static char *get_file_name( char *dirname, char *filename );
static int  get_search_filters( char *filename, char *filters[] );
static int  get_read_entries( char *filename, char *entries[] );
static void fork_child( char *prog, char *args[] );
static void	wait4kids( int nkidval );

static int      maxkids = 20;
static int      nkids;

static void
usage( char *name )
{
	fprintf( stderr, "usage: %s [-h <host>] -p <port> -D <manager> -w <passwd> -d <datadir> -b <baseDN> [-j <maxchild>] [-l <loops>] -P <progdir>\n", name );
	exit( EXIT_FAILURE );
}

int
main( int argc, char **argv )
{
	int		i, j;
	char		*host = "localhost";
	char		*port = NULL;
	char		*manager = NULL;
	char		*passwd = NULL;
	char		*dirname = NULL;
	char        *sbase = NULL;
	char		*progdir = NULL;
	char		*loops = LOOPS;
	DIR			*datadir;
	struct dirent	*file;
	char		*sfile = NULL;
	char		*sreqs[MAXREQS];
	int         snum = 0;
	char		*rfile = NULL;
	char		*rreqs[MAXREQS];
	int         rnum = 0;
	char		*afiles[MAXREQS];
	int         anum = 0;
	char		*sargs[MAXARGS];
	int			sanum;
	char		scmd[MAXPATHLEN];
	char		*rargs[MAXARGS];
	int			ranum;
	char		rcmd[MAXPATHLEN];
	char		*aargs[MAXARGS];
	int			aanum;
	char		acmd[MAXPATHLEN];

	while ( (i = getopt( argc, argv, "h:p:D:w:b:d:j:l:P:" )) != EOF ) {
		switch( i ) {
			case 'h':		/* slapd host */
				host = strdup( optarg );
			break;

			case 'p':		/* the servers port number */
				port = strdup( optarg );
				break;

			case 'D':		/* slapd manager */
				manager = strdup( optarg );
			break;

			case 'w':		/* the managers passwd */
				passwd = strdup( optarg );
				break;

			case 'b':		/* the base DN */
				sbase = strdup( optarg );
				break;

			case 'd':		/* data directory */
				dirname = strdup( optarg );
			break;

			case 'P':		/* prog directory */
				progdir = strdup( optarg );
			break;

			case 'j':		/* the number of parallel clients */
				maxkids = atoi( optarg );
				break;

			case 'l':		/* the number of loops per client */
				loops = strdup( optarg );
				break;

			default:
				usage( argv[0] );
				break;
		}
	}

	if (( dirname == NULL ) || ( sbase == NULL ) || ( port == NULL ) ||
			( manager == NULL ) || ( passwd == NULL ) || ( progdir == NULL ))
		usage( argv[0] );

	/* get the file list */
	if ( ( datadir = opendir( dirname )) == NULL ) {

		fprintf( stderr, "%s: couldn't open data directory \"%s\".\n",
					argv[0], dirname );
		exit( EXIT_FAILURE );

	}

	/*  look for search, read, and add/delete files */
	for ( file = readdir( datadir ); file; file = readdir( datadir )) {

		if ( !strcasecmp( file->d_name, TSEARCHFILE )) {
			sfile = get_file_name( dirname, file->d_name );
			continue;
		} else if ( !strcasecmp( file->d_name, TREADFILE )) {
			rfile = get_file_name( dirname, file->d_name );
			continue;
		} else if ( !strncasecmp( file->d_name, TADDFILE, strlen( TADDFILE ))
			&& ( anum < MAXREQS )) {
			afiles[anum++] = get_file_name( dirname, file->d_name );
			continue;
		}
	}

	closedir( datadir );

	/* look for search requests */
	if ( sfile ) {
		snum = get_search_filters( sfile, sreqs );
	}

	/* look for read requests */
	if ( rfile ) {
		rnum = get_read_entries( rfile, rreqs );
	}

	/*
	 * generate the search clients
	 */

	sanum = 0;
	sprintf( scmd, "%s%s%s", progdir, LDAP_DIRSEP, SEARCHCMD );
	sargs[sanum++] = scmd;
	sargs[sanum++] = "-h";
	sargs[sanum++] = host;
	sargs[sanum++] = "-p";
	sargs[sanum++] = port;
	sargs[sanum++] = "-b";
	sargs[sanum++] = sbase;
	sargs[sanum++] = "-l";
	sargs[sanum++] = loops;
	sargs[sanum++] = "-f";
	sargs[sanum++] = NULL;		/* will hold the search request */
	sargs[sanum++] = NULL;

	/*
	 * generate the read clients
	 */

	ranum = 0;
	sprintf( rcmd, "%s%s%s", progdir, LDAP_DIRSEP, READCMD );
	rargs[ranum++] = rcmd;
	rargs[ranum++] = "-h";
	rargs[ranum++] = host;
	rargs[ranum++] = "-p";
	rargs[ranum++] = port;
	rargs[ranum++] = "-l";
	rargs[ranum++] = loops;
	rargs[ranum++] = "-e";
	rargs[ranum++] = NULL;		/* will hold the read entry */
	rargs[ranum++] = NULL;

	/*
	 * generate the add/delete clients
	 */

	aanum = 0;
	sprintf( acmd, "%s%s%s", progdir, LDAP_DIRSEP, ADDCMD );
	aargs[aanum++] = acmd;
	aargs[aanum++] = "-h";
	aargs[aanum++] = host;
	aargs[aanum++] = "-p";
	aargs[aanum++] = port;
	aargs[aanum++] = "-D";
	aargs[aanum++] = manager;
	aargs[aanum++] = "-w";
	aargs[aanum++] = passwd;
	aargs[aanum++] = "-l";
	aargs[aanum++] = loops;
	aargs[aanum++] = "-f";
	aargs[aanum++] = NULL;		/* will hold the add data file */
	aargs[aanum++] = NULL;

	for ( j = 0; j < MAXREQS; j++ ) {

		if ( j < snum ) {

			sargs[sanum - 2] = sreqs[j];
			fork_child( scmd, sargs );

		}

		if ( j < rnum ) {

			rargs[ranum - 2] = rreqs[j];
			fork_child( rcmd, rargs );

		}

		if ( j < anum ) {

			aargs[aanum - 2] = afiles[j];
			fork_child( acmd, aargs );

		}

	}

	wait4kids( -1 );

	exit( EXIT_SUCCESS );
}

static char *
get_file_name( char *dirname, char *filename )
{
	char buf[MAXPATHLEN];

	sprintf( buf, "%s%s%s", dirname, LDAP_DIRSEP, filename );
	return( strdup( buf ));
}


static int
get_search_filters( char *filename, char *filters[] )
{
	FILE    *fp;
	int     filter = 0;

	if ( (fp = fopen( filename, "r" )) != NULL ) {
		char  line[BUFSIZ];

		while (( filter < MAXREQS ) && ( fgets( line, BUFSIZ, fp ))) {
			char *nl;

			if (( nl = strchr( line, '\r' )) || ( nl = strchr( line, '\n' )))
				*nl = '\0';
			filters[filter++] = strdup( line );

		}
		fclose( fp );
	}

	return( filter );
}


static int
get_read_entries( char *filename, char *entries[] )
{
	FILE    *fp;
	int     entry = 0;

	if ( (fp = fopen( filename, "r" )) != NULL ) {
		char  line[BUFSIZ];

		while (( entry < MAXREQS ) && ( fgets( line, BUFSIZ, fp ))) {
			char *nl;

			if (( nl = strchr( line, '\r' )) || ( nl = strchr( line, '\n' )))
				*nl = '\0';
			entries[entry++] = strdup( line );

		}
		fclose( fp );
	}

	return( entry );
}


static void
fork_child( char *prog, char *args[] )
{
	pid_t	pid;

	wait4kids( maxkids );

	switch ( pid = fork() ) {
	case 0:		/* child */
		execvp( prog, args );
		fprintf( stderr, "%s: ", prog );
		perror( "execv" );
		exit( EXIT_FAILURE );
		break;

	case -1:	/* trouble */
		fprintf( stderr, "Could not fork to run %s\n", prog );
		perror( "fork" );
		break;

	default:	/* parent */
		nkids++;
		break;
	}
}

static void
wait4kids( int nkidval )
{
	int		status;

	while ( nkids >= nkidval ) {
		wait( &status );

		if ( WIFSTOPPED(status) ) {
			fprintf( stderr,
			    "stopping: child stopped with signal %d\n",
			    (int) WSTOPSIG(status) );

		} else if ( WIFSIGNALED(status) ) {
			fprintf( stderr, 
			    "stopping: child terminated with signal %d%s\n",
			    (int) WTERMSIG(status),
#ifdef WCOREDUMP
				WCOREDUMP(status) ? ", core dumped" : ""
#else
				""
#endif
				);
			exit( WEXITSTATUS(status)  );

		} else if ( WEXITSTATUS(status) != 0 ) {
			fprintf( stderr, 
			    "stopping: child exited with status %d\n",
			    (int) WEXITSTATUS(status) );
			exit( WEXITSTATUS(status) );

		} else {
			nkids--;
		}
	}
}
