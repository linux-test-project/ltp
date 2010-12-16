/*
 *  $Id: tst-raw-filter.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-raw-filter.c
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

#define MAXFILTERS 32

int main(int argc, char **argv)
{
        int s;
        struct sockaddr_can addr;
        struct can_filter rfilter[MAXFILTERS];
        struct can_frame frame;
        int nbytes, i;
        struct ifreq ifr;
        char *ifname = "any";
        int ifindex;
        int opt;
        int peek = 0;
        int nfilters = 0;
        int deflt = 0;

        while ((opt = getopt(argc, argv, "i:p:f:d")) != -1) {
                switch (opt) {
                case 'i': /* specify different interface than default */
                        ifname = optarg;
                        break;
                case 'p': /* MSG_PEEK 'p' times before consuming the frame */
                        peek = atoi(optarg);
                        break;
                case 'd': /* use default settings from CAN_RAW socket */
                        deflt = 1;
                        break;
                case 'f': /* add this filter can_id:can_mask */
                        if (nfilters >= MAXFILTERS) {
                                fputs("too many filters\n", stderr);
                                break;
                        }
                        rfilter[nfilters].can_id = strtoul(strtok(optarg, ":"), NULL, 16);
                        rfilter[nfilters].can_mask = strtoul(strtok(NULL, ":"), NULL, 16);
                        nfilters++;
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

        if (deflt) {
                printf("%s: using CAN_RAW socket default filter.\n", argv[0]);
        } else {
                printf("%s: setting %d CAN filter(s).\n", argv[0], nfilters);
                setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter,
                           sizeof(*rfilter) * nfilters);
        }

        if (strcmp(ifname, "any") == 0)
                ifindex = 0;
        else {
                strcpy(ifr.ifr_name, ifname);
                ioctl(s, SIOCGIFINDEX, &ifr);
                ifindex = ifr.ifr_ifindex;
        }

        addr.can_family = AF_CAN;
        addr.can_ifindex = ifindex;

        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("bind");
                return 1;
        }

        while (1) {
                socklen_t len = sizeof(addr);
                int flags;

                if (peek && peek--)
                        flags = MSG_PEEK;
                else
                        flags = 0;

                nbytes = recvfrom(s, &frame, sizeof(struct can_frame),
                                  flags, (struct sockaddr*)&addr, &len);
                if (nbytes < 0) {
                        perror("read");
                        return 1;
                } else if (nbytes < sizeof(struct can_frame)) {
                        fprintf(stderr, "read: incomplete CAN frame from iface %d\n",
                                addr.can_ifindex);
                        return 1;
                } else {
                        ifr.ifr_ifindex = addr.can_ifindex;
                        ioctl(s, SIOCGIFNAME, &ifr);
                        printf(" %-5s ", ifr.ifr_name);
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
                        if (flags & MSG_PEEK)
                                printf(" (MSG_PEEK)");
                        printf("\n");
                        fflush(stdout);
                }
        }

        close(s);

    return 0;
}