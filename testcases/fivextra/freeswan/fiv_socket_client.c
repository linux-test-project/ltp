/******************************************************************************/
/*                                                                            */
/* (C) Copyright IBM Corp. 2003                 			      */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but        */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY */
/* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   */
/* for more details.                                                          */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software		      */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*									      */
/******************************************************************************/

/*
 * client.c -- a stream socket client demo
 * 
 * Author:	Robert Paulsen
 *
 * ToDo:	Make MYPORT and MAXDATASIZE command-line arguments.
 *
 * Change History:
 * 	06 Nov 2003	(rcp) created; from various examples found on the web.
 * 	13 Nov 2003	(rcp) default to localhost
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MYPORT 3490		/* the port client will be connecting to */
#define MAXDATASIZE 4096	/* max number of bytes we can get at once */

int
main(int argc, char *argv[])
{
	int	sockfd;
	int	numbytes;  
	char	buf[MAXDATASIZE];
	char	default_host[]="localhost";
	char	*hostname=default_host;
	struct	hostent *he;
	struct	sockaddr_in their_addr;	/* connector's address information */
	struct	timeval tv;
	fd_set	rfds;

	/* validate/process cmd line args */
	if (argc > 2) {
		fprintf(stderr,"usage: %s [hostname]\n",argv[0]);
		fprintf(stderr,"where hostname defaults to localhost\n");
		return 1;
	}
	if (argc == 2)
		hostname=argv[1];
	if ((he=gethostbyname(hostname)) == NULL) {	/* get the host info */
		perror("gethostbyname");
		return 2;
	}

	/* open and connect to socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return 3;
	}
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(MYPORT);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);
	if (connect(sockfd, (struct sockaddr *)&their_addr,
				sizeof(struct sockaddr)) == -1) {
		perror("connect");
		close(sockfd);
		return 4;
	}

	/* read data, polling for it each time (with 2-second timeout) */
	for (;;) {
		FD_ZERO(&rfds);
		FD_SET(sockfd,&rfds);
		tv.tv_sec=2; tv.tv_usec=0;
		buf[0] = '\0';
		if (select(sockfd+1, &rfds, NULL, NULL, &tv)) {
			if ((numbytes=recv(sockfd,buf,MAXDATASIZE-1,0)) == -1) {
				perror("recv");
				return 5;
			}
			if (numbytes==0) break;
			buf[numbytes] = '\0';
			printf(buf);
		}
		else
			return 6;
	}

	/* all done! */
	close(sockfd);
	return 0;
} 
