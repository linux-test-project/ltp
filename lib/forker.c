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
/**************************************************************
 *
 *    OS Testing - Silicon Graphics, Inc.
 *
 *    FUNCTION NAME     : forker
 *			  background
 *
 *    FUNCTION TITLE    : fork desired number of copies of the current process
 *			  fork a process and return control to caller
 *
 *    SYNOPSIS:
 *      int forker(ncopies, mode, prefix)
 *      int ncopies;
 *	int mode;
 *	char *prefix;
 *
 *	int background(prefix);
 *	char *prefix;
 *
 *	extern int Forker_pids[];
 *	extern int Forker_npids;
 *
 *    AUTHOR            : Richard Logan
 *
 *    CO-PILOT(s)       : Dean Roehrich
 *
 *    INITIAL RELEASE   : UNICOS 8.0
 *
 *    DESIGN DESCRIPTION
 *	The background function will do a fork of the current process.
 *	The parent process will then exit, thus orphaning the
 *	child process.  Doing this will not nice the child process
 *	like executing a cmd in the background using "&" from the shell.
 *	If the fork fails and prefix is not NULL, a error message is printed
 *      to stderr and the process will exit with a value of errno.
 *
 *	The forker function will fork <ncopies> minus one copies
 *	of the current process.  There are two modes in how the forks
 *	will be done.  Mode 0 (default) will have all new processes
 *	be childern of the parent process.    Using Mode 1,
 *	the parent process will have one child and that child will
 *	fork the next process, if necessary, and on and on.
 *	The forker function will return the number of successful
 *	forks.  This value will be different for the parent and each child.
 *	Using mode 0, the parent will get the total number of successful
 *	forks.  Using mode 1, the newest child will get the total number
 *	of forks.  The parent will get a return value of 1.
 *
 *	The forker function also updates the global variables
 *	Forker_pids[] and Forker_npids.  The Forker_pids array will
 *      be updated to contain the pid of each new process.  The
 *	Forker_npids variable contains the number of entries
 *	in Forker_pids.  Note, not all processes will have
 *	access to all pids via Forker_pids.  If using mode 0, only the
 *	parent process and the last process will have all information.
 *      If using mode 1, only the last child process will have all information.
 *
 *	If the prefix parameter is not NULL and the fork system call fails,
 *      a error message will be printed to stderr.  The error message
 *      the be preceeded with prefix string.  If prefix is NULL,
 *      no error message is printed.
 *
 *    SPECIAL REQUIREMENTS
 *	None.
 *
 *    UPDATE HISTORY
 *      This should contain the description, author, and date of any
 *      "interesting" modifications (i.e. info should helpful in
 *      maintaining/enhancing this module).
 *      username     description
 *      ----------------------------------------------------------------
 *	rrl	    This functions will first written during
 *		the SFS testing days, 1993.
 *
 *    BUGS/LIMITATIONS
 *     The child pids are stored in the fixed array, Forker_pids.
 *     The array only has space for 4098 pids.  Only the first
 *     4098 pids will be stored in the array.
 *
 **************************************************************/

#include <stdio.h>
#include <errno.h>
#include <unistd.h> /* fork, getpid, sleep */
#include <string.h>
#include "forker.h"

extern int errno;

int Forker_pids[FORKER_MAX_PIDS];      /* holds pids of forked processes */
int Forker_npids=0;             /* number of entries in Forker_pids */

/***********************************************************************
 *
 * This function will fork and the parent will exit zero and
 * the child will return.  This will orphan the returning process
 * putting it in the background.
 *
 * Return Value
 *   0 : if fork did not fail
 *  !0 : if fork failed, the return value will be the errno.
 ***********************************************************************/
int
background(prefix)
char *prefix;
{
  switch (fork()) {
  case -1:
    if ( prefix != NULL )
        fprintf(stderr, "%s: In %s background(), fork() failed, errno:%d %s\n",
	    prefix, __FILE__, errno, strerror(errno));
    exit(errno);

  case 0:	/* child process */
    break;

  default:	
    exit(0);
  }

  return 0;

}	/* end of background */

/***********************************************************************
 * Forker will fork ncopies-1 copies of self. 
 * 
 ***********************************************************************/
int
forker(ncopies, mode, prefix)
int ncopies;
int mode;	/* 0 - all childern of parent, 1 - only 1 direct child */
char *prefix;   /* if ! NULL, an message will be printed to stderr */
		/* if fork fails.  The prefix (program name) will */
	        /* preceed the message */
{
    int cnt;
    int pid;
    static int ind = 0;

    Forker_pids[ind]=0;

    for ( cnt=1; cnt < ncopies; cnt++ ) {

	switch ( mode ) {
        case 1  :	/* only 1 direct child */
	    if ( (pid = fork()) == -1 ) {
		if ( prefix != NULL ) 
		    fprintf(stderr, "%s: %s,forker(): fork() failed, errno:%d %s\n",
			prefix, __FILE__, errno, strerror(errno));
	        return 0;
	    }
	    Forker_npids++;
	    
	    switch (pid ) {
            case 0:     /* child - continues the forking */
	        
		if ( Forker_npids < FORKER_MAX_PIDS )
                    Forker_pids[Forker_npids-1]=getpid();
                break;

            default:    /* parent - stop the forking */
		if ( Forker_npids < FORKER_MAX_PIDS )
                    Forker_pids[Forker_npids-1]=pid;
                return cnt-1;      
            }

	    break;

	default :	/* all new processes are childern of parent */
	    if ( (pid = fork()) == -1 ) {
		if ( prefix != NULL ) 
		    fprintf(stderr, "%s: %s,forker(): fork() failed, errno:%d %s\n",
			prefix, __FILE__, errno, strerror(errno));
	        return cnt-1;
	    }
	    Forker_npids++;
	    
	    switch (pid ) {
	    case 0:	/* child - stops the forking */
		if ( Forker_npids < FORKER_MAX_PIDS )
                    Forker_pids[Forker_npids-1]=getpid();
	        return cnt;	

	    default:	/* parent - continues the forking */
		if ( Forker_npids < FORKER_MAX_PIDS )
                    Forker_pids[Forker_npids-1]=pid;
                break;
            }
	    break;
	}
    }

    if ( Forker_npids < FORKER_MAX_PIDS )
        Forker_pids[Forker_npids]=0;
    return cnt-1;

}	/* end of forker */


#if UNIT_TEST

/*
 * The following is a unit test main for the background and forker
 * functions.
 */

int
main(argc, argv)
int argc;
char **argv;
{
    int ncopies=1;
    int mode=0;
    int ret;
    int ind;

    if ( argc == 1 ) {
	printf("Usage: %s ncopies [mode]\n", argv[0]);
	exit(1);
    }

    if ( sscanf(argv[1], "%i", &ncopies) != 1 ) {
	printf("%s: ncopies argument must be integer\n", argv[0]);
	exit(1);
    }

    if ( argc == 3 )
	if ( sscanf(argv[2], "%i", &mode) != 1 ) {
        printf("%s: mode argument must be integer\n", argv[0]);
        exit(1);
    }

    printf("Starting Pid = %d\n", getpid());
    ret=background(argv[0]);
    printf("After background() ret:%d, pid = %d\n", ret, getpid());

    ret=forker(ncopies, mode, argv[0]);

    printf("forker(%d, %d, %s) ret:%d, pid = %d, sleeping 30 seconds.\n", 
	ncopies, mode, argv[0], ret, getpid());

    printf("%d My version of Forker_pids[],  Forker_npids = %d\n", 
	getpid(), Forker_npids);

    for (ind=0; ind<Forker_npids; ind++){
	printf("%d ind:%-2d pid:%d\n", getpid(), ind, Forker_pids[ind]);
    }
    
    sleep(30);
    exit(0);
}

#endif  /* UNIT_TEST */
