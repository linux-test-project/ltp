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
 * fivget.c		simple http client
 * Created by Bob Paulsen
 *
 * Added port number for Tomcat - Helen Pang. June 12, 2003
 * Added SSL option (-s flag) - Helen Pang. June 15, 2003
 *
 * 17 July 2003	- fix check for nbr of cmd line args (rcp)
 * 		- removed "-s" hack -- get right from command line (rcp)
 * 15 Oct 2003 - re-wrote the data read section (rcp)
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

#define BUFSZ 16384	/* recv buffer size */

struct	hostent *he;

/* to construct *req.
 *	First %s will be "http" or "https"
 *	Second %s will be host to get from
 *	Third %s filename to get
 */
char	*reqf="GET %s://%s/%s HTTP/1.0\r\n\r\n";

char	*req;	/* request to be sent, constructed from the above */
char	*me;	/* name of this program */
int	port;	/* typically 80; 8080 for tomcat */

void
usage()
{
	fprintf(stderr,"usage: %s proto hostname portnum filename\n",me);
	fprintf(stderr,"where:\tproto is the protocol to use (http or https).\n");
	fprintf(stderr,"where:\thostname is the server to connect to.\n");
	fprintf(stderr,"where:\tportnum is the port to connect to.\n");
	fprintf(stderr,"where:\tfilename is the file to get.\n");
	fprintf(stderr,"Output is to stdout.\n");
	exit(1);
}

void
setup(int argc, char **argv)
{
	char*	proto;

	me=argv[0];
	if (argc != 5) usage();

	/* host info */
	if ((he=gethostbyname(argv[2])) == NULL) {
		perror("gethostbyname");
		usage();
	}

	/* the protocol */
	proto=argv[1];

	/* build request string */
	req=(char*)malloc(1+strlen(argv[1])+strlen(reqf)+strlen(argv[2])+strlen(argv[4]));
	sprintf(req,reqf,argv[1],argv[2],argv[4]);
	port = atoi(argv[3]);
}

int
main(int argc, char **argv)
{
	int	n;
	int	sockfd;
	char	buf[BUFSZ];
	struct	sockaddr_in sin_httpd;		/* http server info */
	struct	timeval tv;
	fd_set	rfds;

	setup(argc,argv);			/* exits if bad */

	/* create socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return 2;
	}

	/* connect to other host */
	sin_httpd.sin_family = AF_INET;		/* host byte order */
	sin_httpd.sin_port = htons(port);	/* network byte order */
	sin_httpd.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(sin_httpd.sin_zero), '\0', 8); /* zero the pad bytes */
	if (connect(sockfd, (struct sockaddr *)&sin_httpd,
			sizeof(struct sockaddr)) == -1) {
		perror("connect");
		close(sockfd);
		return 3;
	}

	/* send request string */
	if ((n=send(sockfd, req, strlen(req), 0)) != strlen(req)) {
		if (n==-1) perror("send");
		else fprintf(stderr,"not all sent\n");
		return 4;
	}

	/* read data, polling for it each time (with timeout) */
	for (;;) {
		FD_ZERO(&rfds);
		FD_SET(sockfd,&rfds);
		tv.tv_sec=10; tv.tv_usec=0;
		buf[0] = '\0';
		if (select(sockfd+1, &rfds, NULL, NULL, &tv)) {
			if ((n=recv(sockfd,buf,BUFSZ-1,0)) == -1) {
				perror("recv");
				return 5;
			}
			if (n==0) break;
			buf[n] = '\0';
			printf(buf);
		}
		else {
			fprintf(stderr,"timeout (%d seconds) waiting for server\n",
					tv.tv_sec);
			return 6;
		}
	}

	/* all done! */
	close(sockfd);
	return 0;
}
