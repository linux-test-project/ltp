/* -*- linux-c -*- */
/*
 *
 *
 *   Copyright (c) International Business Machines  Corp., 2000
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
 *
 *
 */
/*
 * ltpServer.c
 *
 * LTP Network Socket Test Server
 *
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define LOCAL_UDP_SERVER_PORT   10000
#define LOCAL_TCP_SERVER_PORT   10001
#define LOCAL_MULTI_SERVER_PORT 10002
#define MAX_MSG_LEN             256
#define MAX_HOSTNAME_LEN        256
#define ERROR -1
#define END_LINE                0x0A

int udpSocketHandle,
    rc, msg_bytes, tcpSocketHandle, newTcpSocketHandle, multiSocketHandle;

socklen_t udpClientLen, tcpClientLen, multiClientLen;

struct sockaddr_in udpClientAddr,
    udpServerAddr,
    tcpClientAddr, tcpServerAddr, multiClientAddr, multiServerAddr;

struct ip_mreq multiCastReq;
struct in_addr multiCastAddr;
struct hostent *hostEntry;

char message[MAX_MSG_LEN];
char hostname[MAX_HOSTNAME_LEN];
char ServerProg[MAX_HOSTNAME_LEN];

void *ltp_udp_server_queue(void *);
void *ltp_tcp_server_queue(void *);
void *ltp_multi_server_queue(void *);
int tcp_receive_buffer(int, char *);

int main(int argc, char *argv[])
{

	pthread_t udp_server_queue, tcp_server_queue, multi_server_queue;

	pthread_attr_t udpthread_attr, tcpthread_attr, multithread_attr;

	if (argc != 2) {
		printf
		    ("Server arguments : %s <multiCast I.P.address/hostname>\n",
		     argv[0]);
		exit(0);
	}

	strncpy(hostname, argv[1], 255);
	strncpy(ServerProg, argv[0], 255);

	/* get mcast address to listen to */

	hostEntry = gethostbyname(argv[1]);

	if (hostEntry == NULL) {
		printf("Server %s : You need to pass a multiCast group '%s'\n",
		       argv[0], argv[1]);
		exit(1);
	}

	memcpy(&multiCastAddr, hostEntry->h_addr_list[0], hostEntry->h_length);

	/* check given address is multicast */
	if (!IN_MULTICAST(ntohl(multiCastAddr.s_addr))) {
		printf
		    ("%s : Hostname [%s] passed [%s] is not a multicast server\n",
		     argv[0], hostname, inet_ntoa(multiCastAddr));
		printf("The multiCast Server will not be started \n");
	} else {

		/* create multiCast socket */
		multiSocketHandle = socket(AF_INET, SOCK_DGRAM, 0);

		if (multiSocketHandle < 0) {
			printf("%s : cannot create multiCast socket\n",
			       argv[0]);
		} else {
			/* bind multiCast port */
			multiServerAddr.sin_family = AF_INET;
			multiServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			multiServerAddr.sin_port =
			    htons(LOCAL_MULTI_SERVER_PORT);

			if (bind
			    (multiSocketHandle,
			     (struct sockaddr *)&multiServerAddr,
			     sizeof(multiServerAddr)) < 0) {
				printf("%s : cannot bind Multicast port %d \n",
				       argv[0], LOCAL_MULTI_SERVER_PORT);
			} else {
				/* join multicast group */
				multiCastReq.imr_multiaddr.s_addr =
				    multiCastAddr.s_addr;
				multiCastReq.imr_interface.s_addr =
				    htonl(INADDR_ANY);

				rc = setsockopt(multiSocketHandle, IPPROTO_IP,
						IP_ADD_MEMBERSHIP,
						(void *)&multiCastReq,
						sizeof(multiCastReq));
				if (rc < 0) {
					printf
					    ("%s : cannot join multicast group '%s'",
					     argv[0], inet_ntoa(multiCastAddr));
				} else {
					printf
					    ("%s : listening to mgroup %s:%d\n",
					     argv[0], inet_ntoa(multiCastAddr),
					     LOCAL_MULTI_SERVER_PORT);
				}
			}
		}

		rc = pthread_attr_init(&multithread_attr);
		rc = pthread_create(&multi_server_queue, &multithread_attr,
				    ltp_multi_server_queue, NULL);
	}

	/* udp socket creation */
	udpSocketHandle = socket(AF_INET, SOCK_DGRAM, 0);

	if (udpSocketHandle < 0) {
		printf("%s: cannot open socket \n", argv[0]);
		exit(1);
	}

	/* tcp socket creation */
	tcpSocketHandle = socket(AF_INET, SOCK_STREAM, 0);

	if (tcpSocketHandle < 0) {
		printf("Error: cannot open socket %d \n", tcpSocketHandle);
		return ERROR;
	}

	/* bind local udp server port */
	udpServerAddr.sin_family = AF_INET;
	udpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	udpServerAddr.sin_port = htons(LOCAL_UDP_SERVER_PORT);

	rc = bind(udpSocketHandle, (struct sockaddr *)&udpServerAddr,
		  sizeof(udpServerAddr));

	if (rc < 0) {
		printf("%s: Error binding port number %d \n",
		       argv[0], LOCAL_UDP_SERVER_PORT);
		exit(1);
	} else {
		printf("%s: bound port number %d \n",
		       argv[0], LOCAL_UDP_SERVER_PORT);
	}

	/* bind local tcp server port */
	tcpServerAddr.sin_family = AF_INET;
	tcpServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	tcpServerAddr.sin_port = htons(LOCAL_TCP_SERVER_PORT);

	rc = bind(tcpSocketHandle, (struct sockaddr *)&tcpServerAddr,
		  sizeof(tcpServerAddr));

	if (rc < 0) {
		printf("%s: Error binding port number %d \n",
		       argv[0], LOCAL_TCP_SERVER_PORT);
		exit(1);
	} else {
		printf("%s: bound port number %d \n",
		       argv[0], LOCAL_TCP_SERVER_PORT);
	}

	rc = pthread_attr_init(&udpthread_attr);
	rc = pthread_create(&udp_server_queue, &udpthread_attr,
			    ltp_udp_server_queue, NULL);

	rc = pthread_attr_init(&tcpthread_attr);
	rc = pthread_create(&tcp_server_queue, &tcpthread_attr,
			    ltp_tcp_server_queue, NULL);

	while (1) ;

	return 0;
}

/*
 * Function:     ltp_udp_server_queue
 * Description:  This function grabs the udp message from the queue and outputs to stdio
 */
void *ltp_udp_server_queue(void *junk)
{

	printf("%s: waiting for data on port UDP %u\n",
	       hostname, LOCAL_UDP_SERVER_PORT);

	/* server infinite loop */
	while (1) {

		/* init buffer */
		memset(message, 0, MAX_MSG_LEN);

		/* receive message */
		udpClientLen = sizeof(udpClientAddr);

		msg_bytes =
		    recvfrom(udpSocketHandle, message, MAX_MSG_LEN, 0,
			     (struct sockaddr *)&udpClientAddr, &udpClientLen);

		printf("msg_bytes:%d \n", msg_bytes);

		if (msg_bytes < 0) {
			printf("%s: Error receiving data \n", hostname);
		} else {
			/* print message */
			printf("%s: from %s:UDP%u : %s \n",
			       hostname, inet_ntoa(udpClientAddr.sin_addr),
			       ntohs(udpClientAddr.sin_port), message);
		}

	}			/* end of server infinite loop */

	return NULL;

}

/*
 * Function:     ltp_tcp_server_queue
 * Description:  This function grabs the tcp message from the queue and outputs to stdio
 */
void *ltp_tcp_server_queue(void *junk)
{

	listen(tcpSocketHandle, 5);

	while (1) {

		printf("%s: waiting for data on port TCP %u\n", hostname,
		       LOCAL_TCP_SERVER_PORT);

		tcpClientLen = sizeof(tcpClientAddr);
		newTcpSocketHandle =
		    accept(tcpSocketHandle, (struct sockaddr *)&tcpClientAddr,
			   &tcpClientLen);

		if (newTcpSocketHandle < 0) {
			printf("cannot accept TCP connection ");
			break;
		}

		/* init line */
		memset(message, 0x0, MAX_MSG_LEN);

		/* receive segments */
		while (tcp_receive_buffer(newTcpSocketHandle, message) != ERROR) {

			printf("%s: received from %s:TCP%d : %s\n", hostname,
			       inet_ntoa(tcpClientAddr.sin_addr),
			       ntohs(tcpClientAddr.sin_port), message);

			/* init line */
			memset(message, 0x0, MAX_MSG_LEN);

		}		/* while (read_line) */
		printf("looping in TCP \n");

	}			/* while (1) */

	return NULL;

}

/*
 * Function:     tcp_receive_buffer
 * Description:  This function grabs the message from the tcp queue and
 *               returns it to the calling function in the buffer.
 */
int tcp_receive_buffer(int newSocket, char *return_buffer)
{

	static int bytes_received = 0;
	static char message_received[MAX_MSG_LEN];
	static int count = 0;
	int offset;

	offset = 0;

	while (1) {

		if (bytes_received == 0) {
			/* read data from socket */

			memset(message_received, 0x0, MAX_MSG_LEN);	/* init buffer */

			count =
			    recvfrom(newSocket, message_received, MAX_MSG_LEN,
				     0, (struct sockaddr *)&tcpClientAddr,
				     &tcpClientLen);

			if (count < 0) {
				perror(" cannot receive data ");
				return ERROR;
			} else if (count == 0) {
				printf(" connection closed by client\n");
				close(newSocket);
				if (count) {
				}
				return ERROR;
			}
		}

		/* Check for new data read on socket or */
		/* if still more data in buffer       */

		/* copy line into 'return_buffer' */
		while (*(message_received + bytes_received) != END_LINE
		       && bytes_received < count) {
			memcpy(return_buffer + offset,
			       message_received + bytes_received, 1);
			offset++;
			bytes_received++;
		}

		/* end of line + end of buffer => return line */
		if (bytes_received == count - 1) {
			/* set last byte to END_LINE */
			*(return_buffer + offset) = END_LINE;
			bytes_received = 0;
			return ++offset;
		}

		/* end of line but still some data in buffer => return line */
		if (bytes_received < count - 1) {
			/* set last byte to END_LINE */
			*(return_buffer + offset) = END_LINE;
			bytes_received++;
			return ++offset;
		}

		/* end of buffer but line is not ended => */
		/*  wait for more data to arrive on socket */
		if (bytes_received == count) {
			bytes_received = 0;
			return offset;
		}

	}			/* while */

}

/*
 * Function:     ltp_multi_server_queue
 * Description:  This function grabs the multiCast message from the queue and outputs to stdio
 */
void *ltp_multi_server_queue(void *junk)
{

	printf("%s: waiting for data on port Multicast %u\n",
	       hostname, LOCAL_MULTI_SERVER_PORT);

	/* infinite server loop */
	while (1) {

		multiClientLen = sizeof(multiClientAddr);

		msg_bytes =
		    recvfrom(multiSocketHandle, message, MAX_MSG_LEN, 0,
			     (struct sockaddr *)&multiClientAddr,
			     &multiClientLen);

		if (msg_bytes < 0) {
			printf("%s : cannot receive data\n", hostname);
			continue;
		}

		printf("%s : from %s:%d on %s : %s\n", ServerProg,
		       inet_ntoa(multiClientAddr.sin_addr),
		       ntohs(multiClientAddr.sin_port), hostname, message);

	}			/* end of infinite server loop */

	return NULL;

}
