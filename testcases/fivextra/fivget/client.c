/*
** client.c -- a stream socket client demo
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

#define PORT 3490		/* the port client will be connecting to */
#define MAXDATASIZE 4096	/* max number of bytes we can get at once */

int
main(int argc, char *argv[])
{
	int	num_recvs=0;
	int	sockfd;
	int	numbytes;  
	char	buf[MAXDATASIZE];
	struct	hostent *he;
	struct	sockaddr_in their_addr;	/* connector's address information */
	struct	timeval tv;
	fd_set	rfds;

	/* validate cmd line args */
	if (argc != 2) {
		fprintf(stderr,"usage: %s hostname\n",argv[0]);
		return 1;
	}
	if ((he=gethostbyname(argv[1])) == NULL) {	/* get the host info */
		perror("gethostbyname");
		return 2;
	}

	/* open and connect to socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return 3;
	}
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(PORT);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);
	if (connect(sockfd, (struct sockaddr *)&their_addr,
				sizeof(struct sockaddr)) == -1) {
		perror("connect");
		close(sockfd);
		return 4;
	}

	/* read data, polling for it each time (with timeout) */
	for (;;) {
		FD_ZERO(&rfds);
		FD_SET(sockfd,&rfds);
		tv.tv_sec=10; tv.tv_usec=0;
		buf[0] = '\0';
		if (select(sockfd+1, &rfds, NULL, NULL, &tv)) {
			if ((numbytes=recv(sockfd,buf,MAXDATASIZE-1,0)) == -1) {
				perror("recv");
				return 5;
			}
			if (numbytes==0) break;
			buf[numbytes] = '\0';
			printf(buf);
			++num_recvs;
		}
		else {
			fprintf(stderr,"timeout (%d seconds) waiting for server\n",
					tv.tv_sec);
			return 6;
		}
	}

	close(sockfd);

	printf("%d recv(s)\n",num_recvs);

	return 0;
} 
