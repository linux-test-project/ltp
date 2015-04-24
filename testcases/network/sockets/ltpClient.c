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
 * ltpClient.c
 *
 * LTP Network Socket Test Client
 *
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <resolv.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define LOCAL_UDP_SERVER_PORT   10000
#define LOCAL_TCP_SERVER_PORT   10001
#define LOCAL_MCAST_SERVER_PORT 10002
#define MAX_MSG_LEN             256
#define TIMETOLIVE              10
#define PACKETSIZE	            64
#define NET_ERROR               -1
#define PACKET_LEN              1024	/* 1K should be plenty */
#define TRUE                    1
#define FALSE                   0

struct protoent *protocol = NULL;

struct packet {
	struct icmphdr hdr;
	char msg[PACKETSIZE - sizeof(struct icmphdr)];
};

/*
*   Function Prototypes
*/
int ltp_run_ping_tests(char *hostName);
int ltp_run_traceroute_tests(char *hostName);
void ping_network(struct sockaddr_in *rawAddr, int pid);
void output_to_display(void *netPacket, int bytes, int pid);
unsigned short checksum(void *netPacket, int len);
int network_listener(char *hostname, int pid);
void ltp_traceroute(struct sockaddr_in *rawTraceAddr, char *hostName, int pid);

/*******************************************************************
*  Function: Main
*
*  Main will run the tests in this order.
*       UDP, TCP and Multicast will be run first, if multicast is enabled.
*       If multicast is not enabled, the UDP/TCP tests will continue.
*       Once those tests complete, the ping and then traceroute tests will run.
*
********************************************************************/
int main(int argc, char *argv[])
{

	int udpSocketHandle, tcpSocketHandle, mcastSocketHandle, rc, i;

	struct sockaddr_in udpClientAddr,
	    udpRemoteServerAddr,
	    tcpClientAddr,
	    tcpRemoteServerAddr, mcastClientAddr, mcastRemoteServerAddr;

	struct hostent *hostEntry;

	char hostName[MAX_MSG_LEN],
	    progName[MAX_MSG_LEN], traceName[MAX_MSG_LEN], multiCast = TRUE;

	unsigned char ttl = 1;

	mcastSocketHandle = -1;

	/* check command line args */
	if (argc < 4) {
		printf
		    ("usage :<server-hostname> <trace-hostName> <data1> ... <dataN> \n");
		exit(1);
	}

	strncpy(progName, argv[0], MAX_MSG_LEN);
	strncpy(hostName, argv[1], MAX_MSG_LEN);
	strncpy(traceName, argv[2], MAX_MSG_LEN);

	/* get server IP address (no check if input is IP address or DNS name */
	hostEntry = gethostbyname(hostName);

	if (hostEntry == NULL) {
		printf("%s: unknown host passed'%s' \n", progName, hostName);
		exit(1);
	}

	printf("%s: sending data to '%s' (IP : %s) \n", progName,
	       hostEntry->h_name,
	       inet_ntoa(*(struct in_addr *)hostEntry->h_addr_list[0]));

	/* Setup UDP data packets */

	udpRemoteServerAddr.sin_family = hostEntry->h_addrtype;
	memcpy((char *)&udpRemoteServerAddr.sin_addr.s_addr,
	       hostEntry->h_addr_list[0], hostEntry->h_length);
	udpRemoteServerAddr.sin_port = htons(LOCAL_UDP_SERVER_PORT);

	/* Setup TCP data packets */

	tcpRemoteServerAddr.sin_family = hostEntry->h_addrtype;
	memcpy((char *)&tcpRemoteServerAddr.sin_addr.s_addr,
	       hostEntry->h_addr_list[0], hostEntry->h_length);
	tcpRemoteServerAddr.sin_port = htons(LOCAL_TCP_SERVER_PORT);

	/* Setup multiCast data packets */

	mcastRemoteServerAddr.sin_family = hostEntry->h_addrtype;
	memcpy((char *)&mcastRemoteServerAddr.sin_addr.s_addr,
	       hostEntry->h_addr_list[0], hostEntry->h_length);
	mcastRemoteServerAddr.sin_port = htons(LOCAL_MCAST_SERVER_PORT);

	/* socket creation */
	udpSocketHandle = socket(AF_INET, SOCK_DGRAM, 0);
	tcpSocketHandle = socket(AF_INET, SOCK_STREAM, 0);

	if (udpSocketHandle < 0) {
		printf("%s: Error: cannot open UDP socket \n", progName);
	}

	if (tcpSocketHandle < 0) {
		printf("%s: Error: cannot open TCP socket \n", progName);
	}

	/* bind any UDP port */
	udpClientAddr.sin_family = AF_INET;
	udpClientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	udpClientAddr.sin_port = htons(0);

	/* bind any TCP port */
	tcpClientAddr.sin_family = AF_INET;
	tcpClientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	tcpClientAddr.sin_port = htons(0);

	if (udpSocketHandle > 0) {
		rc = bind(udpSocketHandle, (struct sockaddr *)&udpClientAddr,
			  sizeof(udpClientAddr));

		if (rc < 0) {
			printf("%s: Error: cannot bind UDP port\n", progName);
		}
	}

	if (tcpSocketHandle > 0) {
		rc = bind(tcpSocketHandle, (struct sockaddr *)&tcpClientAddr,
			  sizeof(tcpClientAddr));

		if (rc < 0) {
			printf("%s: Error: cannot bind TCP port\n", progName);
		} else {
			/* connect to server */
			rc = connect(tcpSocketHandle,
				     (struct sockaddr *)&tcpRemoteServerAddr,
				     sizeof(tcpRemoteServerAddr));

			if (rc < 0) {
				printf
				    ("Error: cannot connect tp TCP Server \n");
			}
		}
	}

	/* check given address is multicast */
	if (!IN_MULTICAST(ntohl(mcastRemoteServerAddr.sin_addr.s_addr))) {
		printf
		    ("%s : Hostname [%s] passed [%s] is not a multicast server\n",
		     progName, hostName,
		     inet_ntoa(mcastRemoteServerAddr.sin_addr));
		printf("The multiCast Server will not be started \n");
		multiCast = FALSE;
	} else {
		/* create socket */
		mcastSocketHandle = socket(AF_INET, SOCK_DGRAM, 0);
		if (mcastSocketHandle < 0) {
			printf("Error: %s : cannot open mulitCast socket\n",
			       progName);
			multiCast = FALSE;
		}

		/* bind any port number */
		mcastClientAddr.sin_family = AF_INET;
		mcastClientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		mcastClientAddr.sin_port = htons(0);

		if (bind
		    (mcastSocketHandle, (struct sockaddr *)&mcastClientAddr,
		     sizeof(mcastClientAddr)) < 0) {
			printf("Error: binding multiCast socket");
			multiCast = FALSE;
		}

		if (setsockopt
		    (mcastSocketHandle, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
		     sizeof(ttl)) < 0) {
			printf("Error: %s : cannot set ttl = %d \n", progName,
			       ttl);
			multiCast = FALSE;
		}

		printf("%s : sending data on multicast group '%s' (%s)\n",
		       progName, hostEntry->h_name,
		       inet_ntoa(*(struct in_addr *)hostEntry->h_addr_list[0]));

	}

	/* Skip over the program and hostnames and just send data */
	for (i = 3; i < argc; i++) {

		if (udpSocketHandle > 0) {
			rc = sendto(udpSocketHandle, argv[i],
				    strlen(argv[i]) + 1, 0,
				    (struct sockaddr *)&udpRemoteServerAddr,
				    sizeof(udpRemoteServerAddr));

			if (rc < 0) {
				printf("%s: cannot send UDP data %d \n",
				       progName, i - 1);
				close(udpSocketHandle);
			} else {
				printf("%s: UDP data%u sent (%s)\n", progName,
				       i - 1, argv[i]);
			}
		} else {
			printf("%s UDP Socket not open for send \n", hostName);
		}

		if (tcpSocketHandle > 0) {
			rc = send(tcpSocketHandle, argv[i], strlen(argv[i]) + 1,
				  0);

			if (rc < 0) {
				printf("cannot send TCP data ");
				close(tcpSocketHandle);

			} else {
				printf("%s: TCP data%u sent (%s)\n", progName,
				       i - 1, argv[i]);
			}
		} else {
			printf("%s TCP Socket not open for send \n", hostName);
		}

		if (multiCast) {
			rc = sendto(mcastSocketHandle, argv[i],
				    strlen(argv[i]) + 1, 0,
				    (struct sockaddr *)&mcastRemoteServerAddr,
				    sizeof(mcastRemoteServerAddr));

			if (rc < 0) {
				printf("%s : cannot send multiCast data %d\n",
				       progName, i - 1);
				close(mcastSocketHandle);
				multiCast = FALSE;
			}
		}
	}

	sleep(5);

	ltp_run_traceroute_tests(traceName);

	ltp_run_ping_tests(hostName);

	return 0;

}

/*****************************************************************************
* Function: ltp_run_traceroute_tests - host look up and start traceroute processes
*
******************************************************************************/
int ltp_run_traceroute_tests(char *hostName)
{

	struct hostent *hostEntry;
	struct sockaddr_in rawTraceAddr;
	int pid = -1;

	pid = getpid();

	protocol = getprotobyname("ICMP");
	hostEntry = gethostbyname(hostName);

	memset(&rawTraceAddr, 0, sizeof(rawTraceAddr));

	rawTraceAddr.sin_family = hostEntry->h_addrtype;
	rawTraceAddr.sin_port = 0;
	rawTraceAddr.sin_addr.s_addr = *(long *)hostEntry->h_addr;

	ltp_traceroute(&rawTraceAddr, hostName, pid);

	return 0;
}

/**********************************************************************
* Function: ltp_run_ping_tests - host look up and start ping processes
*
***********************************************************************/
int ltp_run_ping_tests(char *hostName)
{

	struct hostent *hostEntry;
	struct sockaddr_in rawAddr;
	int pid = -1;

	pid = getpid();

	protocol = getprotobyname("ICMP");
	hostEntry = gethostbyname(hostName);

	memset(&rawAddr, 0, sizeof(rawAddr));

	rawAddr.sin_family = hostEntry->h_addrtype;
	rawAddr.sin_port = 0;
	rawAddr.sin_addr.s_addr = *(long *)hostEntry->h_addr;

	if (fork() == 0) {
		network_listener(hostName, pid);
	} else {
		ping_network(&rawAddr, pid);

	}

	return 0;
}

/******************************************************************************
* Function: network_listener - separate process to listen for and collect messages
*
*******************************************************************************/
int network_listener(char *hostName, int pid)
{

	int rawSocket, count, value = TIMETOLIVE;
	struct sockaddr_in rawAddr;
	unsigned char packet[PACKET_LEN];

	rawSocket = socket(PF_INET, SOCK_RAW, protocol->p_proto);
	count = 0;

	if (rawSocket < 0) {
		printf("%s: Error: cannot open RAW socket \n", hostName);
		return (NET_ERROR);
	}

	while (1) {		/* loop forever */

		int bytes;
		socklen_t len = sizeof(rawAddr);

		memset(packet, 0, sizeof(packet));

		bytes =
		    recvfrom(rawSocket, packet, sizeof(packet), 0,
			     (struct sockaddr *)&rawAddr, &len);

		if (bytes > 0)
			output_to_display(packet, bytes, pid);
		else {
			printf("%s : cannot receive data\n", hostName);
			break;
		}
		count++;

		if (value == count) {
			printf("Exiting the network_listener...\n");
		}
	}

	close(rawSocket);
	return (0);
}

/****************************************************************
* Function: checksum - standard 1s complement checksum
*
*****************************************************************/
unsigned short checksum(void *netPacket, int len)
{

	unsigned short *packetPtr = netPacket, result;

	unsigned int sum = 0;

	for (sum = 0; len > 1; len -= 2) {
		sum += *packetPtr++;
	}

	if (len == 1) {
		sum += *(unsigned char *)packetPtr;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);

	result = ~sum;

	return result;
}

/*****************************************************************
* Function: output_to_display - Output to display info. from the
*                               listener
******************************************************************/
void output_to_display(void *netPacket, int bytes, int pid)
{

	int i;
	struct iphdr *ip = netPacket;
	struct icmphdr *icmpPtr = netPacket + ip->ihl * 4;
	struct in_addr tmp_addr;

	printf
	    ("\n************** -- Ping Tests - **********************************************\n");

	for (i = 0; i < bytes; i++) {
		if (!(i & 15)) {
			printf("\n[%d]:  ", i);
		}

		printf("[%d] ", ((unsigned char *)netPacket)[i]);
	}

	printf("\n");

	tmp_addr.s_addr = ip->saddr;

	printf("IPv%d: hdr-size=%d pkt-size=%d protocol=%d TTL=%d src=%s ",
	       ip->version, ip->ihl * 4, ntohs(ip->tot_len), ip->protocol,
	       ip->ttl, inet_ntoa(tmp_addr));

	tmp_addr.s_addr = ip->daddr;
	printf("dst=%s\n", inet_ntoa(tmp_addr));

	if (icmpPtr->un.echo.id == pid) {

		printf("ICMP: type[%d/%d] checksum[%d] id[%d] seq[%d]\n\n",
		       icmpPtr->type, icmpPtr->code, ntohs(icmpPtr->checksum),
		       icmpPtr->un.echo.id, icmpPtr->un.echo.sequence);

	}
}

/***********************************************************************
* Function: ping_network - Build a message and send it.
*
*
***********************************************************************/
void ping_network(struct sockaddr_in *rawAddr, int pid)
{

	const int value = TIMETOLIVE;
	int i, rawSocket, count = 1;

	struct packet rawPacket;

	struct sockaddr_in r_addr;

	rawSocket = socket(PF_INET, SOCK_RAW, protocol->p_proto);

	if (rawSocket < 0) {
		printf("Error: cannot open RAW socket %d\n", rawSocket);
		return;
	}

	if (setsockopt(rawSocket, SOL_IP, IP_TTL, &value, sizeof(value)) != 0) {
		printf("ERROR: Setting TimeToLive option");
	} else {
		printf
		    ("The test will run for [%d] iterations -- Ctrl-C to interupt \n",
		     value);
		sleep(3);
	}

	if (fcntl(rawSocket, F_SETFL, O_NONBLOCK) != 0) {
		printf("ERROR: Failed request nonblocking I/O");
	}

	while (1) {

		socklen_t msgLength = sizeof(r_addr);

		printf("Message ID #:%d \n", count);

		if (recvfrom
		    (rawSocket, &rawPacket, sizeof(rawPacket), 0,
		     (struct sockaddr *)&r_addr, &msgLength) > 0) {
			printf("*** -- Message Received -- ***\n");
		}

		memset(&rawPacket, 0, sizeof(rawPacket));

		rawPacket.hdr.type = ICMP_ECHO;
		rawPacket.hdr.un.echo.id = pid;

		for (i = 0; i < sizeof(rawPacket.msg) - 1; i++) {
			rawPacket.msg[i] = i + '0';
		}

		rawPacket.msg[i] = 0;
		rawPacket.hdr.un.echo.sequence = count++;
		rawPacket.hdr.checksum =
		    checksum(&rawPacket, sizeof(rawPacket));

		if (sendto
		    (rawSocket, &rawPacket, sizeof(rawPacket), 0,
		     (struct sockaddr *)rawAddr, sizeof(*rawAddr)) <= 0)
			printf("ERROR: sendto failed !!");

		sleep(1);

		if (value == count) {
			printf("Exiting ping test...\n");
			break;
		}
	}

	close(rawSocket);
}

/**********************************************************************
*  Function: ltp_traceroute
*                      try to reach the destination
*                      while outputting hops along the route
***********************************************************************/
void ltp_traceroute(struct sockaddr_in *rawTraceAddr, char *hostName, int pid)
{

	const int flag = TRUE;
	int TimeToLive = 0;
	int i, rawTraceSocket, count = 1;
	socklen_t length;
	struct packet rawTracePacket;
	unsigned char tracePacket[PACKET_LEN];
	struct sockaddr_in rawReceiveAddr;
	struct hostent *hostEntry2;
	struct in_addr tmp_addr;

	printf
	    ("\n************** -- Trace Route Tests - **********************************************\n");

	rawTraceSocket = socket(PF_INET, SOCK_RAW, protocol->p_proto);

	if (rawTraceSocket < 0) {
		printf("Error: cannot open RAW socket %d\n", rawTraceSocket);
		return;
	}

	if (setsockopt(rawTraceSocket, SOL_IP, SO_ERROR, &flag, sizeof(flag)) !=
	    0)
		printf("ERROR: Setting socket options");

	do {
		struct iphdr *ip;
		length = sizeof(rawReceiveAddr);

		TimeToLive++;
		if (setsockopt
		    (rawTraceSocket, SOL_IP, IP_TTL, &TimeToLive,
		     sizeof(TimeToLive)) != 0) {
			printf("ERROR: Setting TimeToLive option");
		}

		memset(&rawTracePacket, 0, sizeof(rawTracePacket));

		rawTracePacket.hdr.type = ICMP_ECHO;
		rawTracePacket.hdr.un.echo.id = pid;

		for (i = 0; i < sizeof(rawTracePacket.msg) - 1; i++) {
			rawTracePacket.msg[i] = i + '0';
		}

		rawTracePacket.msg[i] = 0;
		rawTracePacket.hdr.un.echo.sequence = count++;
		rawTracePacket.hdr.checksum =
		    checksum(&rawTracePacket, sizeof(rawTracePacket));

		if (sendto
		    (rawTraceSocket, &rawTracePacket, sizeof(rawTracePacket), 0,
		     (struct sockaddr *)rawTraceAddr,
		     sizeof(*rawTraceAddr)) <= 0) {
			printf("ERROR: sendto failed !!");
		}
		sleep(1);

		if (recvfrom
		    (rawTraceSocket, tracePacket, sizeof(tracePacket),
		     MSG_DONTWAIT, (struct sockaddr *)&rawReceiveAddr,
		     &length) > 0) {
			ip = (void *)tracePacket;

			tmp_addr.s_addr = ip->saddr;
			printf("Host IP:#%d: %s \n", count - 1,
			       inet_ntoa(tmp_addr));

			hostEntry2 =
			    gethostbyaddr((void *)&rawReceiveAddr, length,
					  rawReceiveAddr.sin_family);

			if (hostEntry2 != NULL)
				printf("(%s)\n", hostEntry2->h_name);
			else
				perror("Name: ");
		} else {
			printf("%s : data send complete...\n", hostName);
			break;
		}

	}
	while (rawReceiveAddr.sin_addr.s_addr != rawTraceAddr->sin_addr.s_addr);

	printf
	    ("\n************** -- End Trace Route Tests - ******************************************\n");

	close(rawTraceSocket);
}
