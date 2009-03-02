/*
 *  $Id: tst-bcm-tx-sendto.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-bcm-tx-sendto.c
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

#define U64_DATA(p) (*(unsigned long long*)(p)->data)

int main(int argc, char **argv)
{
        int s;
        struct sockaddr_can addr;
        struct ifreq ifr;

        struct {
                struct bcm_msg_head msg_head;
                struct can_frame frame;
        } txmsg;

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

        txmsg.msg_head.opcode  = TX_SETUP;
        txmsg.msg_head.can_id  = 0x42;
        txmsg.msg_head.flags   = SETTIMER|STARTTIMER;
        txmsg.msg_head.nframes = 1;
        txmsg.msg_head.count = 10;
        txmsg.msg_head.ival1.tv_sec = 1;
        txmsg.msg_head.ival1.tv_usec = 0;
        txmsg.msg_head.ival2.tv_sec = 0;
        txmsg.msg_head.ival2.tv_usec = 0;
        txmsg.frame.can_id    = 0x42;
        txmsg.frame.can_dlc   = 8;
        U64_DATA(&txmsg.frame) = (__u64) 0xdeadbeefdeadbeefULL;

        /* should cause an error due to ifindex = 0 */
        if (write(s, &txmsg, sizeof(txmsg)) < 0)
                perror("write");

        printf("Press any key to send on valid device ...\n");
        getchar();

        addr.can_family = PF_CAN;
        strcpy(ifr.ifr_name, "vcan2");
        ioctl(s, SIOCGIFINDEX, &ifr);
        addr.can_ifindex = ifr.ifr_ifindex;

        if (sendto(s, &txmsg, sizeof(txmsg), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0)
                perror("sendto");

        printf("Press any key to close the socket ...\n");
        getchar();

        close(s);

        return 0;
}

