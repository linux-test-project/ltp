/******************************************************************************/
/*                                                                            */
/*   Copyright (c) International Business Machines  Corp., 2005               */
/*                                                                            */
/*   This program is free software;  you can redistribute it and/or modify    */
/*   it under the terms of the GNU General Public License as published by     */
/*   the Free Software Foundation; either version 2 of the License, or        */
/*   (at your option) any later version.                                      */
/*                                                                            */
/*   This program is distributed in the hope that it will be useful,          */
/*   but WITHOUT ANY WARRANTY;  without even the implied warranty of          */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                */
/*   the GNU General Public License for more details.                         */
/*                                                                            */
/*   You should have received a copy of the GNU General Public License        */
/*   along with this program;  if not, write to the Free Software             */
/*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  */
/*                                                                            */
/******************************************************************************/


/*
 * File:
 *	ns-tcpserver.c
 *
 * Description:
 *	Accept connections from the clients, then send tcp segments to clients
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Oct 19 2005 - Created (Mitsuru Chinen)
 *---------------------------------------------------------------------------*/

#define NS_COMMON 1
#include "ns-traffic.h"

/*
 * Fixed values
 */
#define PROC_RMEM_MAX	"/proc/sys/net/core/rmem_max"
#define PROC_WMEM_MAX	"/proc/sys/net/core/wmem_max"

/*
 * Standard Header Files
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>


/*
 * Function: fatal_error()
 *
 * Description:
 *  Output an error message then exit the program with EXIT_FAILURE
 *
 * Argument:
 *  errmsg: message printed by perror()
 *
 * Return value:
 *  This function does not return.
 */
void
fatal_error(char *errmsg)
{
    perror(errmsg);
    exit(EXIT_FAILURE);
}


/*
 * Function: maximize_sockbuf()
 * 
 * Descripton:
 *  This function maximize the send and receive buffer size of a socket
 *
 * Argument:
 *  sd:	target socket descriptor
 *
 * Return value:
 *  None
 */
void
maximize_sockbuf(int sd)
{
    size_t idx;
    int level[] = { SO_RCVBUF, SO_SNDBUF };
    char *procfile[] = { PROC_RMEM_MAX, PROC_WMEM_MAX };
    char *bufname[] = {"rcvbuf", "sndbuf"};

    for (idx = 0; idx < (sizeof(level) / sizeof(int)); idx++) {
	FILE *fp;		/* File pointer to a proc file */
	int bufsiz;		/* buffer size of socket */
	int optlen;		/* size of sd option parameter */

	if ((fp = fopen(procfile[idx], "r")) == NULL) {
	    fprintf(stderr, "Failed to open %s\n", procfile[idx]);
	    fatal_error("fopen()");
	}
	if ((fscanf(fp, "%d", &bufsiz)) != 1) {
	    fprintf(stderr, "Failed to read from %s\n", procfile[idx]);
	    fatal_error("fscanf()");
	}
	if (setsockopt(sd, SOL_SOCKET, level[idx], &bufsiz, sizeof(int))) {
	    fatal_error("setsockopt()");
	}
	if (fclose(fp)) {
	    fprintf(stderr, "Failed to close to %s\n", procfile[idx]);
	    fatal_error("fopen()");
	}

	if (debug) {
	    optlen = sizeof(bufsiz);
	    if (getsockopt(sd, SOL_SOCKET, level[idx], &bufsiz, &optlen) < 0) {
		fatal_error("getsockopt()");
	    }
	    fprintf(stderr, "socket %s size is %d\n", bufname[idx], bufsiz);
	}
    }
}
