/*
 *  $Id: tst-raw.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-raw.c
 *
 * Copyright (c) 2002-2007 Volkswagen Group Electronic Research
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
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int main(int argc, char **argv)
{
        int s;
        struct sockaddr_can addr;
        struct can_filter rfilter[4];
        struct can_frame frame;
        int nbytes, i;
        struct ifreq ifr;
        char *ifname = "vcan2";
        int ifindex;
        int opt;

        /* sockopt test */
        int loopback = 0;
        int set_loopback = 0;
        int recv_own_msgs = 0;
        int set_recv_own_msgs = 0;
        int send_one_frame = 0;
        int ignore_errors = 0;

        while ((opt = getopt(argc, argv, "i:l:r:se")) != -1) {
                switch (opt) {

                case 'i':
                        ifname = optarg;
                        break;

                case 'l':
                        loopback = atoi(optarg);
                        set_loopback = 1;
                        break;

                case 'r':
                        recv_own_msgs = atoi(optarg);
                        set_recv_own_msgs = 1;
                        break;

                case 's':
                        send_one_frame = 1;
                        break;

                case 'e':
                        ignore_errors = 1;
                        break;

                default:
                        fprintf(stderr, "Unknown option %c\n", opt);
                        break;
                }
        }

        if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
                perror("socket");
                return 1;
        }

        rfilter[0].can_id   = 0x123;
        rfilter[0].can_mask = CAN_SFF_MASK;
        rfilter[1].can_id   = 0x200;
        rfilter[1].can_mask = 0x700;
        rfilter[2].can_id   = 0x80123456;
        rfilter[2].can_mask = 0x1FFFF000;
        rfilter[3].can_id   = 0x80333333;
        rfilter[3].can_mask = CAN_EFF_MASK;

        setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

        if (set_loopback)
                setsockopt(s, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));

        if (set_recv_own_msgs)
                setsockopt(s, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs, sizeof(recv_own_msgs));

        strcpy(ifr.ifr_name, ifname);
        ioctl(s, SIOCGIFINDEX, &ifr);
        ifindex = ifr.ifr_ifindex;

        addr.can_family = AF_CAN;
        addr.can_ifindex = ifindex;

        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("bind");
                return 1;
        }

        if (send_one_frame) {

                frame.can_id  = 0x123;
                frame.can_dlc = 2;
                frame.data[0] = 0x11;
                frame.data[1] = 0x22;

                nbytes = write(s, &frame, sizeof(struct can_frame));
        }

        while (1) {

                if ((nbytes = read(s, &frame, sizeof(struct can_frame))) < 0) {
                        perror("read");
                        if (!ignore_errors)
                                return 1;
                } else if (nbytes < sizeof(struct can_frame)) {
                        fprintf(stderr, "read: incomplete CAN frame\n");
                        return 1;
                } else {
                        if (frame.can_id & CAN_EFF_FLAG)
                                printf("%8X  ", frame.can_id & CAN_EFF_MASK);
                        else
                                printf("%3X  ", frame.can_id & CAN_SFF_MASK);

                        printf("[%d] ", frame.can_dlc);

                        for (i = 0; i < frame.can_dlc; i++) {
                                printf("%02X ", frame.data[i]);
                        }
                        if (frame.can_id & CAN_RTR_FLAG)
                                printf("remote request");
                        printf("\n");
                        fflush(stdout);
                }
        }

        close(s);

    return 0;
}