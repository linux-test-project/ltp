/*
 * fivget.c		Write to a port
 * Oct 09 3004 - Created by Bob Paulsen
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

struct	hostent *he;

char	*me;	/* name of this program */
int	port;	/* port number */

void usage()
{
	fprintf(stderr,"usage: %s hostname portnum\n",me);
	fprintf(stderr,"where:\thostname is the server to connect to.\n");
	fprintf(stderr,"where:\tportnum is the port to connect to.\n");
	exit(1);
}

void setup(int argc, char **argv)
{
	me=argv[0];
	if (argc != 3) usage();

	/* host info */
	if ((he=gethostbyname(argv[1])) == NULL) {
		perror("gethostbyname");
		usage();
	}

	port = atoi(argv[2]);
}

int main(int argc, char **argv)
{
	int	n;
	int	sockfd;
	char	*req="Hello!\n";
	struct	sockaddr_in server;		/* server info */

	setup(argc,argv);			/* exits if bad */

	/* create socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	/* connect to host */
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(server.sin_zero), '\0', 8);		/* zero the pad bytes */
	if (connect(sockfd, (struct sockaddr *)&server,
			sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}

	/* send request string */
	if ((n=send(sockfd, req, strlen(req), 0)) != strlen(req)) {
		if (n==-1) perror("send");
		else printf("not all sent\n");
		exit(1);
	}

	/* all done! */
	close(sockfd);
	return 0;
}
