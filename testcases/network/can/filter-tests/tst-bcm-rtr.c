/*
 *  $Id: tst-bcm-rtr.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-bcm-rtr.c
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

#define RTR_SETUP

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
        strcpy(ifr.ifr_name, "vcan2");
        ioctl(s, SIOCGIFINDEX, &ifr);
        addr.can_ifindex = ifr.ifr_ifindex;

        if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("connect");
                return 1;
        }

#ifdef RTR_SETUP
        /* specify CAN-Frame to send as reply to a RTR-request */
        txmsg.msg_head.opcode  = RX_SETUP;
        txmsg.msg_head.can_id  = 0x359 | CAN_RTR_FLAG;
        txmsg.msg_head.flags   = RX_RTR_FRAME; /* | TX_CP_CAN_ID */;
        txmsg.msg_head.ival1.tv_sec = 0; /* no timers in RTR-mode */
        txmsg.msg_head.ival1.tv_usec = 0;
        txmsg.msg_head.ival2.tv_sec = 0;
        txmsg.msg_head.ival2.tv_usec = 0;
        txmsg.msg_head.nframes = 1; /* exact 1 */

        /* the frame to send as reply ... */
        txmsg.frame.can_id = 0x359; /* 'should' be the same */
        txmsg.frame.can_dlc = 3;
        txmsg.frame.data[0] = 0x12;
        txmsg.frame.data[1] = 0x34;
        txmsg.frame.data[2] = 0x56;

#else
        /* normal receiption of RTR-frames in Userspace */
        txmsg.msg_head.opcode  = RX_SETUP;
        txmsg.msg_head.can_id  = 0x359 | CAN_RTR_FLAG;
        txmsg.msg_head.flags   = RX_FILTER_ID;
        txmsg.msg_head.ival1.tv_sec = 0;
        txmsg.msg_head.ival1.tv_usec = 0;
        txmsg.msg_head.ival2.tv_sec = 0;
        txmsg.msg_head.ival2.tv_usec = 0;
        txmsg.msg_head.nframes = 0;
#endif

        if (write(s, &txmsg, sizeof(txmsg)) < 0)
                perror("write");

        while (1) {

                if ((nbytes = read(s, &rxmsg, sizeof(rxmsg))) < 0)
                        perror("read");

                ioctl(s, SIOCGSTAMP, &tv);
                printf("(%ld.%06ld)   ", tv.tv_sec, tv.tv_usec);

                if (rxmsg.msg_head.opcode == RX_CHANGED &&
                    nbytes == sizeof(struct bcm_msg_head) + sizeof(struct can_frame) &&
                    (rxmsg.msg_head.can_id & CAN_SFF_MASK) == 0x359 &&
                    (rxmsg.frame.can_id & CAN_SFF_MASK) == 0x359) {
                        printf("RX_CHANGED message for can_id <%03X> RTR = %d\n",
                               rxmsg.frame.can_id, (rxmsg.frame.can_id & CAN_RTR_FLAG)?1:0);
                }
        }

        close(s);

    return 0;
}