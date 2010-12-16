/*
 *  $Id: tst-err.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-err.c
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
#include <linux/can/error.h>

int main(int argc, char **argv)
{
        int s;
        struct sockaddr_can addr;
        struct can_filter rfilter;
        struct can_frame frame;
        can_err_mask_t err_mask = CAN_ERR_MASK; /* all */
        int nbytes;
        struct ifreq ifr;
        char *ifname = "vcan2";
        int ifindex;
        int opt;
        struct timeval tv;

        while ((opt = getopt(argc, argv, "i:m:")) != -1) {
                switch (opt) {
                case 'i':
                        ifname = optarg;
                        break;
                case 'm':
                        err_mask = strtoul(optarg, (char **)NULL, 16);
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

        rfilter.can_id   = CAN_INV_FILTER; /* no normal CAN frames */
        rfilter.can_mask = 0; /* all: INV(all) == nothing */
        setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

        setsockopt(s, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));

        strcpy(ifr.ifr_name, ifname);
        ioctl(s, SIOCGIFINDEX, &ifr);
        ifindex = ifr.ifr_ifindex;

        addr.can_family = AF_CAN;
        addr.can_ifindex = ifindex;

        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("bind");
                return 1;
        }

        while (1) {

                if ((nbytes = read(s, &frame, sizeof(struct can_frame))) < 0) {
                        perror("read");
                        return 1;
                } else if (nbytes < sizeof(struct can_frame)) {
                        fprintf(stderr, "read: incomplete CAN frame\n");
                        return 1;
                } else {
                        if (ioctl(s, SIOCGSTAMP, &tv) < 0)
                                perror("SIOCGSTAMP");
                        else
                                printf("(%ld.%06ld) ", tv.tv_sec, tv.tv_usec);

                        if (frame.can_id & CAN_ERR_BUSOFF)
                                printf("(bus off) ");

                        if (frame.can_id & CAN_ERR_TX_TIMEOUT)
                                printf("(tx timeout) ");

                        if (frame.can_id & CAN_ERR_ACK)
                                printf("(ack) ");

                        if (frame.can_id & CAN_ERR_LOSTARB) {
                                printf("(lost arb)");
                                if (frame.data[0])
                                        printf("[%d]", frame.data[0]);
                                printf(" ");
                        }

                        if (frame.can_id & CAN_ERR_CRTL) {
                                printf("(crtl)");
                                if (frame.data[1] & CAN_ERR_CRTL_RX_OVERFLOW)
                                        printf("[RX buffer overflow]");
                                if (frame.data[1] & CAN_ERR_CRTL_TX_OVERFLOW)
                                        printf("[TX buffer overflow]");
                                if (frame.data[1] & CAN_ERR_CRTL_RX_WARNING)
                                        printf("[RX warning]");
                                if (frame.data[1] & CAN_ERR_CRTL_TX_WARNING)
                                        printf("[TX warning]");
                                printf(" ");
                        }

                        /* to be continued */

                        printf("\n");
                        fflush(stdout);
                }
        }

        close(s);

    return 0;
}