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
 * server.c -- a stream socket server demo
 * 
 * Author:	Robert Paulsen
 *
 * ToDo:	Make MYPORT and BACKLOG command-line arguments.
 *
 * Change History:
 * 	06 Nov 2003	(rcp) created; from various examples found on the web.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MYPORT 3490	/* the port users will be connecting to */

#define BACKLOG 10	/* how many pending connections queue will hold */

void sigchld_handler(int s)
{
	while(wait(NULL) > 0);
}

int main(void)
{
	int sock_fd, new_fd;	/* listen on sock_fd, new connect on new_fd */
	struct sockaddr_in my_addr;	/* my address information */
	struct sockaddr_in their_addr;	/* connector's address information */
	int sin_size;
	struct sigaction sa;
	int yes=1;

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if (setsockopt(sock_fd,
			SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;		/* host byte order */
	my_addr.sin_port = htons(MYPORT);	/* short, network byte order */
	my_addr.sin_addr.s_addr = INADDR_ANY;	/* auto-fill with my IP */
	memset(&(my_addr.sin_zero), '\0', 8);	/* zero rest of the struct */

	if (bind(sock_fd, (struct sockaddr *)&my_addr,
			sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sock_fd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler;	/* reap all dead processes */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	while(1) {	/* main accept() loop */
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sock_fd, (struct sockaddr *)&their_addr,
				&sin_size)) == -1) {
			perror("accept");
			continue;
		}
		/* printf("server: got connection from %s\n",
			inet_ntoa(their_addr.sin_addr)); */
		if (!fork()) {		/* this is the child process */
			close(sock_fd);	/* child doesn't need the listener */
			if (send(new_fd, "Hello, world!\n", 14, 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);		/* parent doesn't need this */
	}

	return 0;
} 
