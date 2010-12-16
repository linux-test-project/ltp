/*
 *  $Id: tst-packet.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-packet.c - send and receive CAN frames via PF_PACKET sockets
 *
 * Remark: The sending of CAN frames via PF_PACKET does not work for
 * virtual CAN devices (vcan) as the packet type is not set to
 * PACKET_LOOPBACK by the packet socket, so you'll never get something
 * back (btw the outgoing vcan packet counters are updated correctly).
 *
 * Copyright (c) 2002-2009 Volkswagen Group Electronic Research
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
#include <netinet/in.h>         /* for htons() */

#include <linux/if_packet.h>
#include <linux/if_ether.h>     /* for ETH_P_CAN */
#include <linux/can.h>          /* for struct can_frame */

int main(int argc, char **argv)
{
        int s;
        struct can_frame frame;
        int nbytes, i;
        static struct ifreq ifr;
        static struct sockaddr_ll sll;
        char *ifname = "vcan2";
        int ifindex;
        int opt;
        int send_one_frame = 0;

        while ((opt = getopt(argc, argv, "i:s")) != -1) {
                switch (opt) {

                case 'i':
                        ifname = optarg;
                        break;

                case 's':
                        send_one_frame = 1;
                        break;

                default:
                        fprintf(stderr, "Unknown option %c\n", opt);
                        break;
                }
        }

        s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CAN));
        if (s < 0) {
                perror("socket");
                return 1;
        }

        if (strcmp(ifname, "any") == 0)
                ifindex = 0;
        else {
                strcpy(ifr.ifr_name, ifname);
                ioctl(s, SIOCGIFINDEX, &ifr);
                ifindex = ifr.ifr_ifindex;
        }

        sll.sll_family   = AF_PACKET;
        sll.sll_ifindex  = ifindex;
        sll.sll_protocol = htons(ETH_P_CAN);

        if (bind(s, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
                perror("bind");
                return 1;
        }

        if (send_one_frame) {

                frame.can_id  = 0x123;
                frame.can_dlc = 2;
                frame.data[0] = 0x11;
                frame.data[1] = 0x22;

                nbytes = write(s, &frame, sizeof(struct can_frame));
                if (nbytes < 0) {
                        perror("write");
                        return 1;
                }
                if (nbytes != sizeof(struct can_frame)) {
                        perror("write_len");
                        return 1;
                }
        }

        while (1) {

                if ((nbytes = read(s, &frame, sizeof(struct can_frame))) < 0) {
                        perror("read");
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