/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
*   File: pthserv.c	Version: 1.3		Last update: 5/19/94 08:55:35
*/
/******************************************************************************/
/* File:        pthserv.c                                                     */
/*                                                                            */
/* Description: Read a stream socket one line at a time and write each line   */
/*              back to the sender.                                           */
/*                                                                            */
/* History:     Contact - 06/21/2001 - Manoj Iyeri, IBM Austin                */
/*                                                                            */
/* Usage:       pthcli [port number]                                          */
/*                                                                            */
/******************************************************************************/

/*
    TCP server
*/

#include <pthread.h>
#include <string.h>
#include "inet.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define MAXLINE 1024
void noprintf(char *string, ...)
{
}

pthread_t th;
pthread_mutex_t current_mutex;
int sockfd;

/* Read a stream socket one line at a time and write each line back
   to sender. Return when connection is terminated */

int str_echo(int sockfd)
{
	int n, testint;
	char line[MAXLINE];

	printf("sockfd = %d\n", sockfd);
	for (;;) {
		prtln();
		dprt2(("%s: str_echo(): reading from sockfd %d\n", __FILE__,
		       sockfd));
		n = readline(sockfd, line, MAXLINE);
		printf("str_echo: n = %d\n", n);
		if (n == 0) {
			dprt2(("%s: str_echo(): connection terminated\n",
			       __FILE__));
			return 0;	/* connection terminated */
		} else if (n < 0) {
			perror("str_echo: readline error");
			return (-1);
		}
		dprt2(("%s: str_echo(): writing to sockfd = %d\n", __FILE__,
		       sockfd));
		testint = writen(sockfd, line, n);
		prtln();
		if (testint != n) {
			perror("str_echo: writen error");
			return (-1);
		}
		prtln();
	}
}

int main(int argc, char *argv[])
{
	void *new_thread(void *);
	pthread_attr_t newattr;
	int newsockfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;
	pname = argv[0];

	(void) argc;
	prtln();
	/* Open inet stream socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("server: socket failure:");
		exit(1);
	}
	prtln();
	dprt2(("%s: main(): Open inet stream socket sockfd = %d\n", __FILE__,
	       sockfd));

	/* Bind local address for client to use */
	memset((char *)&serv_addr, 0x00, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_TCP_PORT);
	prtln();
	dprt2(("%s: main(): Binding local address for client to use\n"
	       "serv_addr.sin_family = %d\n serv_addr.sin_addr.s_addr = %#x\n"
	       "serv_addr.sin_port = %d\n", __FILE__, serv_addr.sin_family,
	       serv_addr.sin_addr.s_addr, serv_addr.sin_port));

	prtln();
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("server bind failure:\n");
		fflush(NULL);
		exit(1);
	}

	prtln();
	if (pthread_mutex_init(&current_mutex, NULL) != 0)
		printf("current_mutex_init() failure");
	prtln();

	/* attr init, detached state create thread */
	if (pthread_attr_init(&newattr))
		printf("failure to init attributes\n");
	if (pthread_attr_setdetachstate(&newattr, PTHREAD_CREATE_DETACHED))
		printf("failure to set detached state\n");
	prtln();
	listen(sockfd, 5);

	prtln();
	for (;;) {
		/* Wait for connection from a client process */
		clilen = sizeof(cli_addr);

		newsockfd =
		    accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		prtln();
		if (newsockfd < 0) {
			perror("server: accept");
			printf("server: accept error");
			exit(1);
		} else {	/* create thread to handle client request */

			if (pthread_create(&th, &newattr, new_thread,
					   (void *)(uintptr_t) newsockfd))
				printf("failure to create thread\n");
#ifndef _LINUX
			yield();
#else
			sched_yield();
#endif
		}
		prtln();
	}
	close(sockfd);
}

void *new_thread(void *arg_)
{
	int arg = (uintptr_t) arg_;
	if (pthread_mutex_lock(&current_mutex))
		printf("mutex_lock failed");
	if (str_echo(arg) < 0)	/* process the request */
		printf("new_thread: str_echo returned error");
	close(arg);		/* i.e. newsockfd */
	if (pthread_mutex_unlock(&current_mutex))
		printf("mutex_unlock failed");
#ifndef _LINUX
	yield();
#else
	sched_yield();
#endif
	pthread_exit(NULL);
}
