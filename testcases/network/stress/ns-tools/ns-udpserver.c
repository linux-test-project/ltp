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
/*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA  */
/*                                                                            */
/******************************************************************************/

/*
 * File:
 *	ns-udpserver.c
 *
 * Description:
 *	This is UDP traffic server.
 *	Received UDP datagram from a client, then send it to the client
 *
 * Author:
 *	Mitsuru Chinen <mitch@jp.ibm.com>
 *
 * History:
 *	Oct 19 2005 - Created (Mitsuru Chinen)
 *---------------------------------------------------------------------------*/

#include "ns-traffic.h"

/*
 * Standard Include Files
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>

/*
 * Gloval variables
 */
struct sigaction handler;	/* Behavior for a signal */
int catch_sighup;		/* When catch the SIGHUP, set to non-zero */

/*
 * Function: usage()
 *
 * Descripton:
 *  Print the usage of this program. Then, terminate this program with
 *  the specified exit value.
 *
 * Argument:
 *  exit_value:	exit value
 *
 * Return value:
 *  This function does not return.
 */
void usage(char *program_name, int exit_value)
{
	FILE *stream = stdout;	/* stream where the usage is output */

	if (exit_value == EXIT_FAILURE)
		stream = stderr;

	fprintf(stream, "%s [OPTION]\n"
		"\t-f\tprotocol family\n"
		"\t\t  4 : IPv4\n"
		"\t\t  6 : IPv6\n"
		"\t-p\tport number\n"
		"\t\tthe port number specified by -p option would be the first port number\n"
		"\t-b\twork in the background\n"
		"\t-o\tfilename where the server infomation is outputted\n"
		"\t-d\twork in the debug mode\n"
		"\t-h\tdisplay this usage\n"
		"" "*) Server works till it receives SIGHUP\n", program_name);
	exit(exit_value);
}

/*
 * Function: set_signal_flag()
 *
 * Description:
 *  This function sets global variable according to the signal.
 *  Once a signal is caught, the signal is ignored after that.
 *
 * Argument:
 *  type: type of signal
 *
 * Return value:
 *  None
 */
void set_signal_flag(int type)
{
	/* Set SIG_IGN against the caught signal */
	handler.sa_handler = SIG_IGN;
	if (sigaction(type, &handler, NULL) < 0)
		fatal_error("sigaction()");

	if (debug)
		fprintf(stderr, "Catch signal. type is %d\n", type);

	switch (type) {
	case SIGHUP:
		catch_sighup = 1;
		break;
	default:
		fprintf(stderr, "Unexpected signal (%d) is caught\n", type);
		exit(EXIT_FAILURE);
	}
}

/*
 * Function: respond_to_client()
 *
 * Description:
 *  Recieve the client data. Then, return the data to client.
 *
 * Argument:
 *  sock_fd:	socket file descriptor
 *
 * Return value:
 *  None
 */
void respond_to_client(int sock_fd)
{
	char *msgbuf;		/* Pointer to the message */
	size_t msgbuf_size;	/* size of msgbuf */
	ssize_t msglen;		/* the length of message */
	socklen_t sock_optlen;	/* size of the result parameter */
	struct sockaddr_storage client_addr;	/* address of a client */
	socklen_t client_addr_len;	/* length of `client_addr' */

	sock_optlen = sizeof(sock_optlen);
	if (getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF,
		       &msgbuf_size, &sock_optlen) < 0) {
		perror("getsockopt()");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

	/* Allocate the memory for a message */
	msgbuf = malloc(msgbuf_size + 1);
	if (msgbuf == NULL) {
		fprintf(stderr, "malloc() is failed.\n");
		close(sock_fd);
		exit(EXIT_FAILURE);
	}

	/* Receive a message */
	client_addr_len = sizeof(client_addr);
	if ((msglen = recvfrom(sock_fd, msgbuf, msgbuf_size, 0,
			       (struct sockaddr *)&client_addr,
			       &client_addr_len)) < 0)
		fatal_error("recvfrom()");
	msgbuf[msglen] = '\0';

	if (debug)
		fprintf(stderr, "Message is %s\n", msgbuf);

	/* Return the message to the client */
	if (sendto(sock_fd, msgbuf, msglen, 0,
		   (struct sockaddr *)&client_addr,
		   sizeof(client_addr)) != msglen)
		fatal_error("sendto()");
	free(msgbuf);
}

/*
 *
 *  Function: main()
 *
 */
int main(int argc, char *argv[])
{
	char *program_name = argv[0];
	int optc;		/* option */
	sa_family_t family;	/* protocol family */
	char *portnum = NULL;	/* port number */
	int sock_fd;		/* socket binded open ports */
	int background = 0;	/* work in the background if non-zero */
	fd_set read_fds;	/* list of file descriptor for reading */
	int max_read_fd = 0;	/* maximum number in the read fds */
	FILE *info_fp = stdout;	/* FILE pointer to a information file */
	int on;			/* on/off at an socket option */
	int err;		/* return value of getaddrinfo */
	struct addrinfo hints;	/* hints for getaddrinfo() */
	struct addrinfo *res;	/* pointer to addrinfo */

	debug = 0;
	family = PF_UNSPEC;

	/* Retrieve the options */
	while ((optc = getopt(argc, argv, "f:p:bo:dh")) != EOF) {
		switch (optc) {
		case 'f':
			if (strncmp(optarg, "4", 1) == 0)
				family = PF_INET;	/* IPv4 */
			else if (strncmp(optarg, "6", 1) == 0)
				family = PF_INET6;	/* IPv6 */
			else {
				fprintf(stderr,
					"protocol family should be 4 or 6.\n");
				usage(program_name, EXIT_FAILURE);
			}
			break;

		case 'p':
			{
				unsigned long int num;
				num = strtoul(optarg, NULL, 0);
				if (num < PORTNUMMIN || PORTNUMMAX < num) {
					fprintf(stderr,
						"The range of port is from %u to %u\n",
						PORTNUMMIN, PORTNUMMAX);
					usage(program_name, EXIT_FAILURE);
				}
				portnum = strdup(optarg);
			}
			break;

		case 'b':
			background = 1;
			break;

		case 'o':
			if ((info_fp = fopen(optarg, "w")) == NULL) {
				fprintf(stderr, "Cannot open %s\n", optarg);
				exit(EXIT_FAILURE);
			}
			break;

		case 'd':
			debug = 1;
			break;

		case 'h':
			usage(program_name, EXIT_SUCCESS);
			break;

		default:
			usage(program_name, EXIT_FAILURE);
		}
	}

	/* Check the family is spefied. */
	if (family == PF_UNSPEC) {
		fprintf(stderr, "protocol family should be specified.\n");
		usage(program_name, EXIT_FAILURE);
	}

	/* At first, SIGHUP is ignored. */
	handler.sa_handler = SIG_IGN;
	if (sigfillset(&handler.sa_mask) < 0)
		fatal_error("sigfillset()");
	handler.sa_flags = 0;

	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigaction()");

	/* Set the hints to addrinfo() */
	memset(&hints, '\0', sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	/* Translate the network and service information of the server */
	err = getaddrinfo(NULL, portnum, &hints, &res);
	if (err) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}
	if (res->ai_next) {
		fprintf(stderr, "getaddrinfo(): multiple address is found.");
		exit(EXIT_FAILURE);
	}

	/* Create a socket for listening. */
	sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock_fd < 0)
		fatal_error("socket()");

#ifdef IPV6_V6ONLY
	/* Don't accept IPv4 mapped address if the protocol family is IPv6 */
	if (res->ai_family == PF_INET6) {
		on = 1;
		if (setsockopt
		    (sock_fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(int)))
			fatal_error("setsockopt()");
	}
#endif

	/* Enable to reuse the socket */
	on = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)))
		fatal_error("setsockopt()");

	/* Bind to the local address */
	if (bind(sock_fd, res->ai_addr, res->ai_addrlen) < 0)
		fatal_error("bind()");

	freeaddrinfo(res);

	/* If -b option is specified, work as a daemon */
	if (background)
		if (daemon(0, 0) < 0)
			fatal_error("daemon()");

	/* Output any server information to the information file */
	fprintf(info_fp, "PID: %u\n", getpid());
	fflush(info_fp);
	if (info_fp != stdout)
		if (fclose(info_fp))
			fatal_error("fclose()");

	/* Catch SIGHUP */
	handler.sa_handler = set_signal_flag;
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigaction()");

	/* Loop to wait a client access */
	FD_ZERO(&read_fds);
	FD_SET(sock_fd, &read_fds);
	max_read_fd = sock_fd;
	for (;;) {
		int select_ret;	/* return value of select() */
		fd_set active_fds;	/* list of the active file descriptor */
		struct timeval select_timeout;	/* timeout for select() */

		/* When catch SIGHUP, exit the loop */
		if (catch_sighup)
			break;

		active_fds = read_fds;
		select_timeout.tv_sec = 0;	/* 0.5 sec */
		select_timeout.tv_usec = 500000;

		select_ret = select(max_read_fd + 1, &active_fds,
				    NULL, NULL, &select_timeout);
		if (select_ret < 0) {
			if (catch_sighup)
				break;
			else
				fatal_error("select()");
		} else if (select_ret == 0) {
			continue;
		} else {
			if (FD_ISSET(sock_fd, &active_fds))
				respond_to_client(sock_fd);
		}
	}

	/* Close the sockets */
	if (close(sock_fd))
		fatal_error("close()");

	exit(EXIT_SUCCESS);
}
