/*
 *
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

/******************************************************************************
 *
 *   pthcli.c
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *****************************************************************************/

/******************************************************************************/
/* File:        pthcli.c                                                      */
/*                                                                            */
/* Description: Read contents of data file. Write each line to socket, then   */
/*              ead line back from socket and write to standard output.       */
/*                                                                            */
/*                                                                            */
/* Usage:       pthcli [port number]                                          */
/*                                                                            */
/******************************************************************************/

/* client using TCP */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "inet.h"
#include <errno.h>
#include <stdlib.h>
#define MAXLINE 1024

void noprintf(char *string, ...)
{
	(void) string;
}

/* Read contents of FILE *fp. Write each line to socket, then
   read line back from socket and write to standard output.
   Return to caller when done */

void str_cli(FILE *fp, int sockfd)
{
	int n;
	char sendline[MAXLINE], recvline[MAXLINE + 1];
	prtln();
	while (fgets(sendline, MAXLINE, fp) != NULL) {
		n = strlen(sendline);

		dprt("%s: str_cli(): sendline = %s", __FILE__, sendline);

		if (writen(sockfd, sendline, n) != n)
			perror("str_cli: writen error on socket");
		/*
		 * read a line from socket and write it to standard output
		 */

		prtln();
		n = readline(sockfd, recvline, MAXLINE);
		prtln();
		/*
		   printf("strcli: recvline = %s", recvline);
		 */
		if (n < 0)
			perror("str_cli: readline error on socket");
		recvline[n] = 0;
		fputs(recvline, stdout);
		prtln();
	}

	prtln();
	if (ferror(fp))
		perror("str_cli: error reading file");
}

int main(int argc, char *argv[])
{
	FILE *input;
	int sockfd;
	struct sockaddr_in serv_addr;

	pname = argv[0];
	if (argc < 3) {
		printf("\nusage: %s ip data\n", pname);
		exit(1);
	}

	/* Fill in the structure */
	memset((char *)&serv_addr, 0x00, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(SERV_TCP_PORT);
	prtln();
	dprt("%s: main(): Binding local address for client to use\n"
	     "serv_addr.sin_family = %d\n serv_addr.sin_addr.s_addr = %#x\n"
	     "serv_addr.sin_port = %d\n", __FILE__, serv_addr.sin_family,
	     serv_addr.sin_addr.s_addr, serv_addr.sin_port);

	/* Open Internet stream socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("client: socket open failure, no = %d\n", errno);
		return (errno);
		exit(1);
	}
	prtln();
	dprt("%s: main(): Open Internet stream socket, socfd = %d\n", __FILE__,
	     sockfd);
	/* Connect to the server */
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
	    0) {
		prtln();
		printf("client: connect failure, no = %d\n", errno);
		return (errno);
		exit(1);
	}
#ifdef _LINUX
	if ((input = fopen(argv[2], "r")) == NULL) {
		perror("fopen");
		return (errno);
	}
	str_cli(input, sockfd);	/* call the routines that do the work */
	prtln();
#else
	prtln();
	str_cli(stdin, sockfd);	/* call the routines that do the work */
#endif
	prtln();
	close(sockfd);
	exit(0);
}
