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
 *	ns-tcpserver.c
 *
 * Description:
 *      This is TCP traffic server.
 *	Accept connections from the clients, then send tcp segments to clients
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
#include <netinet/tcp.h>

/*
 * Gloval variables
 */
struct sigaction handler;	/* Behavior for a signal */
int catch_sighup;		/* When catch the SIGHUP, set to non-zero */
int catch_sigpipe;		/* When catch the SIGPIPE, set to non-zero */

/*
 * Structure: server_info
 *
 * Description:
 *  This structure stores the information of a server
 */
struct server_info {
	sa_family_t family;	/* protocol family */
	char *portnum;		/* port number */
	int listen_sd;		/* socket descriptor for listening */
	int concurrent;		/* if non-zero, act as a concurrent server */
	size_t current_connection;	/* number of the current connection */
	size_t max_connection;	/* maximum connection number */
	size_t lost_connection;	/* number of lost connection */
	size_t small_sending;	/* if non-zero, in the small sending mode */
	size_t window_scaling;	/* if non-zero, in the window scaling mode */
};

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
		"\t-b\twork in the background\n"
		"\t-c\twork in the concurrent server mode\n"
		"\t-s\twork in the small sending mode\n"
		"\t-w\twork in the window scaling mode\n"
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
	case SIGPIPE:
		catch_sigpipe = 1;
		break;
	default:
		fprintf(stderr, "Unexpected signal (%d) is caught\n", type);
		exit(EXIT_FAILURE);
	}
}

/*
 * Function: delete_zombies()
 *
 * Descripton:
 *  Delete the zombies
 *
 * Argument:
 *  info_p:	pointer to a server infomation
 *
 * Return value:
 *  None
 */
void delete_zombies(struct server_info *info_p)
{
	int status;		/* exit value of a child */
	pid_t zombie_pid;	/* process id of a zombie */

	while (info_p->current_connection) {
		zombie_pid = waitpid((pid_t) - 1, &status, WNOHANG);
		if (zombie_pid == (pid_t) - 1)
			fatal_error("waitpid()");
		else if (zombie_pid == (pid_t) 0)
			break;
		else {
			--info_p->current_connection;
			if (status != EXIT_SUCCESS) {
				++info_p->lost_connection;
				if (debug)
					fprintf(stderr,
						"The number of lost conncections is %zu\n",
						info_p->lost_connection);
			}
		}
	}
}

/*
 * Function: create_listen_socket()
 *
 * Descripton:
 *  Create a socket to listen for connections on a socket.
 *  The socket discripter is stored info_p->listen_sd.
 *
 * Argument:
 *  info_p:	pointer to a server infomation
 *
 * Return value:
 *  None
 */
void create_listen_socket(struct server_info *info_p)
{
	int on;			/* on/off at an socket option */
	int err;		/* return value of getaddrinfo */
	struct addrinfo hints;	/* hints for getaddrinfo() */
	struct addrinfo *res;	/* pointer to addrinfo */

	/* Set the hints to addrinfo() */
	memset(&hints, '\0', sizeof(struct addrinfo));
	hints.ai_family = info_p->family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	/* Translate the network and service information of the server */
	err = getaddrinfo(NULL, info_p->portnum, &hints, &res);
	if (err) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}
	if (res->ai_next) {
		fprintf(stderr, "getaddrinfo(): multiple address is found.");
		exit(EXIT_FAILURE);
	}

	/* Create a socket for listening. */
	info_p->listen_sd = socket(res->ai_family,
				   res->ai_socktype, res->ai_protocol);
	if (info_p->listen_sd < 0)
		fatal_error("socket()");

#ifdef IPV6_V6ONLY
	/* Don't accept IPv4 mapped address if the protocol family is IPv6 */
	if (res->ai_family == PF_INET6) {
		on = 1;
		if (setsockopt(info_p->listen_sd,
			       IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(int)))
			fatal_error("setsockopt()");
	}
#endif

	/* Enable to reuse the socket */
	on = 1;
	if (setsockopt(info_p->listen_sd,
		       SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)))
		fatal_error("setsockopt()");

	/* Disable the Nagle algorithm, when small sending mode */
	if (info_p->small_sending) {
		on = 1;
		if (setsockopt(info_p->listen_sd,
			       IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int)))
			fatal_error("setsockopt()");
		if (debug) {
			fprintf(stderr, "small sending[on]\n");
		}
	}

	/* Maximize socket buffer, when window scaling mode */
	if (info_p->window_scaling)
		maximize_sockbuf(info_p->listen_sd);

	/* Bind to the local address */
	if (bind(info_p->listen_sd, res->ai_addr, res->ai_addrlen) < 0)
		fatal_error("bind()");
	freeaddrinfo(res);

	/* Start to listen for connections */
	if (listen(info_p->listen_sd, 5) < 0)
		fatal_error("listen()");
}

/*
 * Function: communicate_client()
 *
 * Descripton:
 *  Communicate with the connected client.
 *  Currently, this function sends tcp segment in the specified second
 *  or recevie SIGHUP
 *
 * Argument:
 *  sock_fd: socket descriptor to communicate with client
 *  info_p:  pointer to a server infomation
 *
 * Return value:
 *  0:	    success
 *  other:  fail
 */
int communicate_client(struct server_info *info_p, int sock_fd)
{
	char *sendmsg;		/* pointer to the message to send */
	int sndbuf_size;	/* size of the send buffer */
	socklen_t sock_optlen;	/* size of the result parameter */
	ssize_t sntbyte_size;	/* size of the sent byte */
	int ret = EXIT_SUCCESS;	/* The return value of this function */

	if (info_p->small_sending) {	/* small sending mode */
		sndbuf_size = 1;
	} else {
		sock_optlen = sizeof(sndbuf_size);
		if (getsockopt
		    (sock_fd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size,
		     &sock_optlen) < 0) {
			perror("getsockopt()");
			if (close(sock_fd))
				fatal_error("close()");
			return EXIT_FAILURE;
		}
	}
	if (debug)
		fprintf(stderr, "sndbuf size is %d\n", sndbuf_size);

	/* Define the message */
	sendmsg = malloc(sndbuf_size);
	if (sendmsg == NULL) {
		fprintf(stderr, "malloc() is failed.\n");
		if (close(sock_fd))
			fatal_error("close()");
		return EXIT_FAILURE;
	}

	/* Set a signal handler against SIGHUP and SIGPIPE */
	handler.sa_handler = set_signal_flag;
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigaction()");
	if (sigaction(SIGPIPE, &handler, NULL) < 0)
		fatal_error("sigaction()");

	/* Send the message */
	for (;;) {
		sntbyte_size = send(sock_fd, sendmsg, sndbuf_size, 0);

		/* Catch SIGPIPE */
		if (catch_sigpipe) {
			if (debug)
				fprintf(stderr,
					"The client closed the connection.\n");
			break;
		}

		/* Catch SIGHUP */
		if (catch_sighup)
			break;

		if (sntbyte_size < (ssize_t) 0) {
			if (errno == EPIPE) {
				if (debug)
					fprintf(stderr,
						"The client closed the connection.\n");
			} else {
				printf("errno=%d\n", errno);
				perror("send()");
				ret = EXIT_FAILURE;
			}
			break;
		}
	}

	free(sendmsg);
	if (close(sock_fd))
		fatal_error("close()");
	return ret;
}

/*
 * Function: handle_client()
 *
 * Descripton:
 *  Accept a connection from a client, then fork to communicate the client
 *
 * Argument:
 *  info_p:	pointer to a server infomation
 *
 * Return value:
 *  0:	    success
 *  other:  fail
 */
int handle_client(struct server_info *info_p)
{
	int ret = EXIT_SUCCESS;	/* return value of this function */
	int do_accept = 1;	/* if non-zero, accept connection */
	fd_set read_fds;	/* list of file descriptor for reading */
	int max_read_fd = 0;	/* maximum number in the read fds */

	info_p->current_connection = 0;
	FD_ZERO(&read_fds);
	FD_SET(info_p->listen_sd, &read_fds);
	max_read_fd = info_p->listen_sd;

	/* Catch SIGHUP */
	handler.sa_handler = set_signal_flag;
	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigaction()");

	/* Loop to wait a new connection */
	for (;;) {
		if (do_accept) {
			int data_sd;	/* socket descriptor for send/recv data */
			socklen_t client_addr_len;	/* length of `client_addr' */
			struct sockaddr_storage client_addr;	/* address of a client */
			int select_ret;	/* return value of select() */
			fd_set active_fds;	/* list of the active file descriptor */
			struct timeval select_timeout;	/* timeout for select() */

			/* When catch SIGHUP, no more connection is acceptted. */
			if (catch_sighup) {
				do_accept = 0;
				if (close(info_p->listen_sd))
					fatal_error("close()");
				continue;
			}

			/* Check a connection is requested */
			active_fds = read_fds;
			select_timeout.tv_sec = 0;	/* 0.5 sec */
			select_timeout.tv_usec = 500000;

			select_ret = select(max_read_fd + 1,
					    &active_fds, NULL, NULL,
					    &select_timeout);
			if (select_ret < 0) {
				do_accept = 0;
				if (!catch_sighup) {
					perror("select()");
					ret = EXIT_FAILURE;
				}
				if (close(info_p->listen_sd))
					fatal_error("close()");
				continue;
			} else if (select_ret == 0) {	/* select() is timeout */
				if (info_p->concurrent)
					delete_zombies(info_p);
				continue;
			}

			/* Accetpt a client connection */
			if (FD_ISSET(info_p->listen_sd, &active_fds)) {
				client_addr_len =
				    sizeof(struct sockaddr_storage);
				data_sd =
				    accept(info_p->listen_sd,
					   (struct sockaddr *)&client_addr,
					   &client_addr_len);
				if (data_sd < 0) {
					do_accept = 0;
					if (!catch_sighup) {
						perror("accept()");
						ret = EXIT_FAILURE;
					}
					if (close(info_p->listen_sd))
						fatal_error("close()");
					continue;
				}
				if (debug)
					fprintf(stderr,
						"called accept(). data_sd=%d\n",
						data_sd);

				/* Handle clients */
				if (info_p->concurrent) {	/* concurrent server. */
					pid_t child_pid;
					child_pid = fork();
					if (child_pid < 0) {	/* fork() is failed. */
						perror("fork()");
						if (close(data_sd))
							fatal_error("close()");
						if (close(info_p->listen_sd))
							fatal_error("close()");
						do_accept = 0;
						continue;
					} else if (child_pid == 0) {	/* case of a child */
						int exit_value;
						if (close(info_p->listen_sd))
							fatal_error("close()");
						exit_value =
						    communicate_client(info_p,
								       data_sd);
						if (debug)
							fprintf(stderr,
								"child(%d) exits. value is %d\n",
								getpid(),
								exit_value);
						exit(exit_value);
					} else {	/* case of the parent */
						if (close(data_sd))
							fatal_error("close()");

						++info_p->current_connection;
						if (info_p->max_connection <
						    info_p->
						    current_connection) {
							info_p->max_connection =
							    info_p->
							    current_connection;
							if (debug)
								fprintf(stderr,
									"The maximum connection is updated. The number is %zu.\n",
									info_p->
									max_connection);
						}
						delete_zombies(info_p);
					}
				} else {	/* repeat server */
					ret =
					    communicate_client(info_p, data_sd);
					if (ret != EXIT_SUCCESS)
						if (close(info_p->listen_sd))
							fatal_error("close()");
					break;
				}
			}
		} else {
			/* case where new connection isn't accepted. */
			if (info_p->concurrent)
				delete_zombies(info_p);
			if (info_p->current_connection == 0)
				break;
		}
	}
	return ret;
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
	struct server_info server;	/* server information */
	int ret = EXIT_SUCCESS;	/* exit value */
	int background = 0;	/* If non-zero work in the background */
	FILE *info_fp = stdout;	/* FILE pointer to a information file */

	debug = 0;

	/* Initilalize the server information */
	memset(&server, '\0', sizeof(struct server_info));
	server.family = PF_UNSPEC;
	server.portnum = NULL;

	/* Retrieve the options */
	while ((optc = getopt(argc, argv, "f:p:bcswo:dh")) != EOF) {
		switch (optc) {
		case 'f':
			if (strncmp(optarg, "4", 1) == 0)
				server.family = PF_INET;	/* IPv4 */
			else if (strncmp(optarg, "6", 1) == 0)
				server.family = PF_INET6;	/* IPv6 */
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
				server.portnum = strdup(optarg);
			}
			break;

		case 'b':
			background = 1;
			break;

		case 'c':
			server.concurrent = 1;
			break;

		case 's':
			server.small_sending = 1;
			break;

		case 'w':
			server.window_scaling = 1;
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
	if (server.family == PF_UNSPEC) {
		fprintf(stderr, "protocol family should be specified.\n");
		usage(program_name, EXIT_FAILURE);
	}

	/* Check the port number is specfied. */
	if (server.portnum == NULL) {
		server.portnum = (char *)calloc(6, sizeof(char));
		sprintf(server.portnum, "%u", PORTNUMMIN);
	}

	/* If -b option is specified, work as a daemon */
	if (background)
		if (daemon(0, 0) < 0)
			fatal_error("daemon()");

	/* At first, SIGHUP is ignored. default with SIGPIPE */
	handler.sa_handler = SIG_IGN;
	if (sigfillset(&handler.sa_mask) < 0)
		fatal_error("sigfillset()");
	handler.sa_flags = 0;

	if (sigaction(SIGHUP, &handler, NULL) < 0)
		fatal_error("sigaction()");

	/* Create a listen socket */
	create_listen_socket(&server);

	/* Output any server information to the information file */
	fprintf(info_fp, "PID: %u\n", getpid());
	fflush(info_fp);
	if (info_fp != stdout)
		if (fclose(info_fp))
			fatal_error("fclose()");

	/* Handle one or more tcp clients. */
	ret = handle_client(&server);
	exit(ret);
}
