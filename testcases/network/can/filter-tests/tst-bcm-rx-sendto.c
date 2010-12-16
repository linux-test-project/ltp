/*
 *  $Id: tst-bcm-rx-sendto.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-bcm-rx-sendto.c
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
#include <sys/uio.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/bcm.h>

int main(int argc, char **argv)
{
        int s,nbytes;
        struct sockaddr_can addr;
        struct ifreq ifr;

        struct timeval tv;

        struct {
                struct bcm_msg_head msg_head;
                struct can_frame frame;
        } txmsg, rxmsg;

        if ((s = socket(PF_CAN, SOCK_DGRAM, CAN_BCM)) < 0) {
                perror("socket");
                return 1;
        }

        addr.can_family = PF_CAN;
        addr.can_ifindex = 0; /* bind to 'any' device */

        if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("connect");
                return 1;
        }

        memset(&txmsg, 0, sizeof(txmsg)); /* clear timers, nframes, etc. */

        txmsg.msg_head.opcode  = RX_SETUP;
        txmsg.msg_head.can_id  = 0x123;
        txmsg.msg_head.flags   = RX_FILTER_ID;

        if (write(s, &txmsg, sizeof(txmsg)) < 0)
                perror("write");

        addr.can_family = PF_CAN;
        strcpy(ifr.ifr_name, "vcan2");
        ioctl(s, SIOCGIFINDEX, &ifr);
        addr.can_ifindex = ifr.ifr_ifindex;

        txmsg.msg_head.opcode  = RX_SETUP;
        txmsg.msg_head.can_id  = 0x321;
        txmsg.msg_head.flags   = RX_FILTER_ID;

        if (sendto(s, &txmsg, sizeof(txmsg), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0)
                perror("sendto");

        addr.can_family = PF_CAN;
        strcpy(ifr.ifr_name, "vcan1");
        ioctl(s, SIOCGIFINDEX, &ifr);
        addr.can_ifindex = ifr.ifr_ifindex;

        txmsg.msg_head.opcode  = RX_SETUP;
        txmsg.msg_head.can_id  = 0x424;
        txmsg.msg_head.flags   = RX_FILTER_ID;

        if (sendto(s, &txmsg, sizeof(txmsg), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0)
                perror("sendto");

        while (1) {
                socklen_t len = sizeof(addr);
                nbytes = recvfrom(s, &rxmsg, sizeof(rxmsg),
                                  0, (struct sockaddr*)&addr, &len);
                if (nbytes < 0) {
                        perror("recvfrom");
                        return 1;
                } else if (nbytes < sizeof(rxmsg)) {
                        fprintf(stderr, "recvfrom: incomplete BCM message from iface %d\n",
                                addr.can_ifindex);
                        return 1;
                } else {
                        int i;

                        ioctl(s, SIOCGSTAMP, &tv);
                        printf("(%ld.%06ld) ", tv.tv_sec, tv.tv_usec);

                        ifr.ifr_ifindex = addr.can_ifindex;
                        ioctl(s, SIOCGIFNAME, &ifr);

                        printf(" %-5s ", ifr.ifr_name);
                        if (rxmsg.frame.can_id & CAN_EFF_FLAG)
                                printf("%8X  ", rxmsg.frame.can_id & CAN_EFF_MASK);
                        else
                                printf("%3X  ", rxmsg.frame.can_id & CAN_SFF_MASK);

                        printf("[%d] ", rxmsg.frame.can_dlc);

                        for (i = 0; i < rxmsg.frame.can_dlc; i++) {
                                printf("%02X ", rxmsg.frame.data[i]);
                        }
                        if (rxmsg.frame.can_id & CAN_RTR_FLAG)
                                printf("remote request");
                        printf("\n");
                        fflush(stdout);
                }
        }

        close(s);

    return 0;
}