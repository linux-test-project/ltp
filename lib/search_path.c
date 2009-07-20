/* $Header: /cvsroot/ltp/ltp/lib/search_path.c,v 1.4 2009/07/20 10:59:32 vapier Exp $ */

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
 */


/**********************************************************
 *
 *    UNICOS Feature Test and Evaluation - Cray Research, Inc.
 *
 *    FUNCTION NAME 	: search_path
 *
 *    FUNCTION TITLE	: search PATH locations for desired filename
 *
 *    SYNOPSIS:
 *	int search_path(cmd, res_path, access_mode, fullpath)
 *	char *cmd;
 *	char *res_path;
 *	int access_mode;
 *	int fullpath;
 *
 *    AUTHOR		: Richard Logan
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    DESCRIPTION
 *	Search_path will walk through PATH and attempt to find "cmd".  If cmd is
 *	a full or relative path, it is checked but PATH locations are not scanned.
 *	search_path will put the resulting path in res_path.  It is assumed
 *	that res_path points to a string that is at least PATH_MAX
 *	(or MAXPATHLEN on the suns) in size.  Access_mode is just as is
 *	says, the mode to be used on access to determine if cmd can be found.
 *	If fullpath is set, res_path will contain the full path to cmd.
 *	If it is not set, res_path may or may not contain the full path to cmd.
 *	If fullpath is not set, the path in PATH prepended to cmd is used,
 *	which could be a relative path.  If fullpath is set, the current
 *	directory is prepended to path/cmd before access is called.
 *	If cmd is found, search_path will return 0.  If cmd cannot be
 *	found, 1 is returned.  If an error has occurred, -1 is returned
 *	and an error mesg is placed in res_path.
 *	If the length of path/cmd is larger then PATH_MAX, then that path
 *	location is skipped.
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/stat.h>


struct stat stbuf;

#ifndef AS_CMD
#define AS_CMD	0
#endif

/*
 * Make sure PATH_MAX is defined.  Define it to MAXPATHLEN, if set. Otherwise
 * set it to 1024.
 */
#ifndef PATH_MAX
#ifndef MAXPATHLEN
#define PATH_MAX     1024
#else  /* MAXPATHLEN */
#define PATH_MAX     MAXPATHLEN
#endif  /* MAXPATHLEN */
#endif  /* PATH_MAX */


#if AS_CMD
main(argc, argv)
int argc;
char **argv;
{
    char path[PATH_MAX];
    int ind;

    if (argc <= 1 ) {
	printf("missing argument\n");
	exit(1);
    }

    for(ind=1;ind < argc; ind++) {
	if ( search_path(argv[ind], path, F_OK, 0) < 0 ) {
	    printf("ERROR: %s\n", path);
	}
	else {
	    printf("path of %s is %s\n", argv[ind], path);
	}
    }

}

#endif

/*
 */
int
search_path(cmd, res_path, access_mode, fullpath)
char *cmd;	/* The requested filename */
char *res_path; /* The resulting path or error mesg */
int access_mode; /* the mode used by access(2) */
int fullpath;	/* if set, cwd will be prepended to all non-full paths */
{
    char *cp;   /* used to scan PATH for directories */
    int ret;      /* return value from access */
    char *pathenv;
    char tmppath[PATH_MAX];
    char curpath[PATH_MAX];
    char *path;
    int lastpath;
    int toolong=0;

#if DEBUG
printf("search_path: cmd = %s, access_mode = %d, fullpath = %d\n", cmd, access_mode, fullpath);
#endif

    /*
     * full or relative path was given
     */
    if ( (cmd[0] == '/') || ( (cp=strchr(cmd, '/')) != NULL )) {
	if ( access(cmd, access_mode) == 0 ) {

	    if ( cmd[0] != '/' ) { /* relative path */
		if ( getcwd(curpath, PATH_MAX) == NULL ) {
		    strcpy(res_path, curpath);
		    return -1;
		}
		if ( (strlen(curpath) + strlen(cmd) + 1) > (size_t)PATH_MAX ) {
		    sprintf(res_path, "cmd (as relative path) and cwd is longer than %d",
			PATH_MAX);
		    return -1;
		}
		sprintf(res_path, "%s/%s", curpath, cmd);
	    }
	    else
	        strcpy(res_path, cmd);
	    return 0;
        }
	else {
	    sprintf(res_path, "file %s not found", cmd);
	    return -1;
	}
    }

    /* get the PATH variable */
    if ( (pathenv=getenv("PATH")) == NULL) {
        /* no path to scan, return */
	sprintf(res_path, "Unable to get PATH env. variable");
        return -1;
    }

    /*
     * walk through each path in PATH.
     * Each path in PATH is placed in tmppath.
     * pathenv cannot be modified since it will affect PATH.
     * If a signal came in while we have modified the PATH
     * memory, we could create a problem for the caller.
     */

    curpath[0]='\0';

    cp = pathenv;
    path = pathenv;
    lastpath = 0;
    for (;;) {

	if ( lastpath )
	    break;

	if ( cp != pathenv )
	    path = ++cp;	 /* already set on first iteration */

	/* find end of current path */

	for (; ((*cp != ':') && (*cp != '\0')); cp++);

	/*
	 * copy path to tmppath so it can be NULL terminated
	 * and so we do not modify path memory.
	 */
	strncpy(tmppath, path, (cp-path) );
	tmppath[cp-path]='\0';
#if DEBUG
printf("search_path: tmppath = %s\n", tmppath);
#endif

	if ( *cp == '\0' )
	    lastpath=1;		/* this is the last path entry */

	/* Check lengths so not to overflow res_path */
	if ( strlen(tmppath) + strlen(cmd) + 2 > (size_t)PATH_MAX ) {
	    toolong++;
	    continue;
	}

	sprintf(res_path, "%s/%s", tmppath, cmd);
#if DEBUG
printf("search_path: res_path = '%s'\n", res_path);
#endif


	    /* if the path is not full at this point, prepend the current
	     * path to get the full path.
	     * Note:  this could not be wise to do when under a protected
	     * directory.
	     */

	if ( fullpath && res_path[0] != '/' ) {	/* not a full path */
	    if ( curpath[0] == '\0' ) {
		if ( getcwd(curpath, PATH_MAX) == NULL ) {
                    strcpy(res_path, curpath);
                    return -1;
	 	}
            }
            if ( (strlen(curpath) + strlen(res_path) + 2) > (size_t)PATH_MAX ) {
		toolong++;
	        continue;
            }
            sprintf(tmppath, "%s/%s", curpath, res_path);
	    strcpy(res_path, tmppath);
#if DEBUG
printf("search_path: full res_path= '%s'\n", res_path);
#endif

	}


	if ( (ret=access(res_path, access_mode)) == 0 ) {
#if DEBUG
printf("search_path: found res_path = %s\n", res_path);
#endif
	    return 0;
	}
    }

    /* return failure */
    if ( toolong )
        sprintf(res_path,
	    "Unable to find file, %d path/file strings were too long", toolong);
    else
        strcpy(res_path, "Unable to find file");
    return 1;	/* not found */
}

