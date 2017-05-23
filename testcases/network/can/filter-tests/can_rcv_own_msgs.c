/*
 * tst-rcv-own-msgs.c
 *
 * Copyright (c) 2010 Volkswagen Group Electronic Research
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Volkswagen nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * Alternatively, provided that this notice is retained in full, this
 * software may be distributed under the terms of the GNU General
 * Public License ("GPL") version 2, in which case the provisions of the
 * GPL apply INSTEAD OF those given above.
 *
 * The provided data structures and external interfaces from this code
 * are not restricted to be used by modules with a GPL compatible license.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * Send feedback to <socketcan-users@lists.berlios.de>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include "config.h"
#include "tst_res_flags.h"
#include "tst_minmax.h"

#ifdef HAVE_LINUX_CAN_H

#include <linux/can.h>
#include <linux/can/raw.h>

struct rxs {
	int s;
	int t;
};

struct rxs test_sockets(int s, int t, canid_t can_id)
{
	fd_set rdfs;
	struct timeval tv;
	int m = MAX(s, t) + 1;
	int have_rx = 1;
	struct can_frame frame;
	struct rxs rx;
	int ret;

	frame.can_id = can_id;
	frame.can_dlc = 0;
	if (write(s, &frame, sizeof(frame)) < 0) {
		perror("write");
		exit(1);
	}

	rx.s = rx.t = 0;

	while (have_rx) {

		FD_ZERO(&rdfs);
		FD_SET(s, &rdfs);
		FD_SET(t, &rdfs);
		tv.tv_sec = 0;
		tv.tv_usec = 50000;	/* 50ms timeout */
		have_rx = 0;

		ret = select(m, &rdfs, NULL, NULL, &tv);
		if (ret < 0) {
			perror("select");
			exit(1);
		}

		if (FD_ISSET(s, &rdfs)) {

			have_rx = 1;
			ret = read(s, &frame, sizeof(struct can_frame));
			if (ret < 0) {
				perror("read");
				exit(1);
			}
			if (frame.can_id != can_id) {
				fprintf(stderr, "received wrong can_id!\n");
				exit(1);
			}
			rx.s++;
		}

		if (FD_ISSET(t, &rdfs)) {

			have_rx = 1;
			ret = read(t, &frame, sizeof(struct can_frame));
			if (ret < 0) {
				perror("read");
				exit(1);
			}
			if (frame.can_id != can_id) {
				fprintf(stderr, "received wrong can_id!\n");
				exit(1);
			}
			rx.t++;
		}
	}

	/* timeout */

	return rx;
}

void setopts(int s, int loopback, int recv_own_msgs)
{
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback,
		   sizeof(loopback));
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs,
		   sizeof(recv_own_msgs));

	printf("check loopback %d recv_own_msgs %d ... ", loopback,
	       recv_own_msgs);
}

int main(int argc, char **argv)
{
	int s, t;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct rxs rx;

	/* check command line options */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <device>\n", argv[0]);
		return TFAIL;
	}

	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) {
		perror("socket");
		return TFAIL;
	}
	t = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (t < 0) {
		perror("socket");
		return TFAIL;
	}

	strcpy(ifr.ifr_name, argv[1]);
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		perror("SIOCGIFINDEX");
		return TFAIL;
	}
	addr.can_ifindex = ifr.ifr_ifindex;
	addr.can_family = AF_CAN;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return TFAIL;
	}
	if (bind(t, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return TFAIL;
	}

	printf("Starting PF_CAN frame flow test.\n");
	printf("checking socket default settings ... ");
	rx = test_sockets(s, t, 0x340);
	if (rx.s == 0 && rx.t == 1)
		printf("ok.\n");
	else {
		printf("failure!\n");
		return TFAIL;
	}

	/* check loopback 0 recv_own_msgs 0 */
	setopts(s, 0, 0);
	rx = test_sockets(s, t, 0x341);
	if (rx.s == 0 && rx.t == 0)
		printf("ok.\n");
	else {
		printf("failure!\n");
		return TFAIL;
	}

	/* check loopback 0 recv_own_msgs 1 */
	setopts(s, 0, 1);
	rx = test_sockets(s, t, 0x342);
	if (rx.s == 0 && rx.t == 0)
		printf("ok.\n");
	else {
		printf("failure!\n");
		return TFAIL;
	}

	/* check loopback 1 recv_own_msgs 0 */
	setopts(s, 1, 0);
	rx = test_sockets(s, t, 0x343);
	if (rx.s == 0 && rx.t == 1)
		printf("ok.\n");
	else {
		printf("failure!\n");
		return TFAIL;
	}

	/* check loopback 1 recv_own_msgs 1 */
	setopts(s, 1, 1);
	rx = test_sockets(s, t, 0x344);
	if (rx.s == 1 && rx.t == 1)
		printf("ok.\n");
	else {
		printf("failure!\n");
		return TFAIL;
	}

	printf("PF_CAN frame flow test was successful.\n");

	close(s);
	close(t);

	return TPASS;
}

#else

int main(void)
{
	printf("The linux/can.h was missing upon compilation.\n");
	return TCONF;
}

#endif /* HAVE_LINUX_CAN_H */
