/******************************************************************************/
/*                                                                            */
/* (C) Copyright IBM Corp. 2001                 			      */
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
 * fivput.c		simple http client
 * based on Bob Paulsen's fivget.c
 * created by Helen Pang
 * June 03, 2003
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
 * 	First %s will be host to put to
 * 	Second %s filename to put
 */
char	*reqf="PUT http://%s/%s HTTP/1.0\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s";
char    *content="<html><body>Hello!</body></html>";

/* request to be sent, constructed from the above */
char    *req;

/* name of this program */
char	*me;

void
usage()
{
	fprintf(stderr,"usage: %s hostname filename [content]\n",me);
	fprintf(stderr,"where:\thostname is the server to connect to.\n");
	fprintf(stderr,"where:\tfilename is the file to put.\n");
	fprintf(stderr,"where:\tcontent is the cobntent of the file.\n");
	fprintf(stderr,"Output is to stdout.\n");
	exit(1);
}

void
setup(int argc, char **argv)
{
	me=argv[0];
	if (argc < 3 || argc > 4) usage();

	/* host info */
	if ((he=gethostbyname(argv[1])) == NULL) {
		perror("gethostbyname");
		usage();
	}

	if (argc == 4)
		content = argv[3];

	/* build request string */
	req=(char*)malloc(10+strlen(reqf)+strlen(argv[1])+
				strlen(argv[2])+strlen(content));
	sprintf(req, reqf, argv[1], argv[2], strlen(content), content);
	printf("request header is: %s\n",req);
}

int
main(int argc, char **argv)
{
	int	n;
	int	port=80;
	int	sockfd;
	char	buf[BUFSZ];
	struct	sockaddr_in sin_httpd;		/* http server info */
	struct	timeval tv;
	fd_set	rfds;

	setup(argc,argv);			/* exits if bad */
	printf("Setup done!\n");

	/* create socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return 2;
	}

	/* connect to other host */
	sin_httpd.sin_family = AF_INET;		/* host byte order */
	sin_httpd.sin_port = htons(port);	/* network byte order */
	sin_httpd.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(sin_httpd.sin_zero), '\0', 8);	/* zero the pad bytes */
	if (connect(sockfd, (struct sockaddr *)&sin_httpd,
			sizeof(struct sockaddr)) == -1) {
		perror("connect");
		return 3;
	}

	/* send request string */
	if ((n=send(sockfd, req, strlen(req), 0)) != strlen(req)) {
		if (n==-1) perror("send");
		else printf("not all sent\n");
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
