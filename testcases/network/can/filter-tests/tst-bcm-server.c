/*
 *  $Id: tst-bcm-server.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-bcm-server.c
 *
 * Test programm that implements a socket server which understands ASCII
 * messages for simple broadcast manager frame send commands.
 *
 * < interface command ival_s ival_us can_id can_dlc [data]* >
 *
 * The commands are 'A'dd, 'U'pdate, 'D'elete and 'S'end.
 * e.g.
 *
 * Send the CAN frame 123#1122334455667788 every second on vcan1
 * < vcan1 A 1 0 123 8 11 22 33 44 55 66 77 88 >
 *
 * Send the CAN frame 123#1122334455667788 every 10 usecs on vcan1
 * < vcan1 A 0 10 123 8 11 22 33 44 55 66 77 88 >
 *
 * Send the CAN frame 123#42424242 every 20 msecs on vcan1
 * < vcan1 A 0 20000 123 4 42 42 42 42 >
 *
 * Update the CAN frame 123#42424242 with 123#112233 - no change of timers
 * < vcan1 U 0 0 123 3 11 22 33 >
 *
 * Delete the cyclic send job from above
 * < vcan1 D 0 0 123 0 >
 *
 * Send a single CAN frame without cyclic transmission
 * < can0 S 0 0 123 0 >
 *
 * When the socket is closed the cyclic transmissions are terminated.
 *
 * Authors:
 * Andre Naujoks (the socket server stuff)
 * Oliver Hartkopp (the rest)
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
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>
#include <netinet/in.h>

#include <linux/can.h>
#include <linux/can/bcm.h>

void readmsg(int sock, char *buf, int maxlen) {

        int ptr = 0;

        while (read(sock, buf+ptr, 1) == 1) {

                if (ptr) {
                        if (*(buf+ptr) == '>') {
                                *(buf+ptr+1) = 0;
                                return;
                        }
                        if (++ptr > maxlen-2)
                                ptr = 0;
                }
                else
                        if (*(buf+ptr) == '<')
                                ptr++;
        }

        *buf = 0;
}


int main(int argc, char **argv)
{

        int sl, sa, sc;
        struct sockaddr_in  saddr, clientaddr;
        struct sockaddr_can caddr;
        struct ifreq ifr;
        socklen_t sin_size = sizeof(clientaddr);

        char buf[100];

        struct {
                struct bcm_msg_head msg_head;
                struct can_frame frame;
        } msg;

        if((sl = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("inetsocket");
                exit(1);
        }

        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = htonl(INADDR_ANY);
        saddr.sin_port = htons(28600);

        while(bind(sl,(struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
                printf(".");fflush(NULL);
                usleep(100000);
        }

        if (listen(sl,3) != 0) {
                perror("listen");
                exit(1);
        }

        while (1) { 
                sa = accept(sl,(struct sockaddr *)&clientaddr, &sin_size);
                if (sa > 0 ){

                        if (fork())
                                close(sa);
                        else
                                break;
                }
                else {
                        if (errno != EINTR) {
                                /*
                                 * If the cause for the error was NOT the signal from
                                 * a dying child, than give an error
                                 */
                                perror("accept");
                                exit(1);
                        }
                }
        }

        /* open BCM socket */

        if ((sc = socket(PF_CAN, SOCK_DGRAM, CAN_BCM)) < 0) {
                perror("bcmsocket");
                return 1;
        }

        caddr.can_family = PF_CAN;
        caddr.can_ifindex = 0; /* any device => need for sendto() */

        if (connect(sc, (struct sockaddr *)&caddr, sizeof(caddr)) < 0) {
                perror("connect");
                return 1;
        }

        /* prepare stable settings */
        msg.msg_head.nframes       = 1;
        msg.msg_head.count         = 0;
        msg.msg_head.ival1.tv_sec  = 0;
        msg.msg_head.ival1.tv_usec = 0;

        while (1) {

                char cmd;
                int items;

                readmsg(sa, buf, sizeof(buf));

                // printf("read '%s'\n", buf);

                items = sscanf(buf, "< %6s %c %lu %lu %x %hhu "
                               "%hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx >",
                               ifr.ifr_name,
                               &cmd, 
                               &msg.msg_head.ival2.tv_sec,
                               &msg.msg_head.ival2.tv_usec,
                               &msg.msg_head.can_id,
                               &msg.frame.can_dlc,
                               &msg.frame.data[0],
                               &msg.frame.data[1],
                               &msg.frame.data[2],
                               &msg.frame.data[3],
                               &msg.frame.data[4],
                               &msg.frame.data[5],
                               &msg.frame.data[6],
                               &msg.frame.data[7]);

                if (items < 6)
                        break;
                if (msg.frame.can_dlc > 8)
                        break;
                if (items != 6 + msg.frame.can_dlc)
                        break;

                msg.frame.can_id = msg.msg_head.can_id;

                switch (cmd) {
                case 'S':
                        msg.msg_head.opcode = TX_SEND;
                        break;
                case 'A':
                        msg.msg_head.opcode = TX_SETUP;
                        msg.msg_head.flags |= SETTIMER|STARTTIMER;
                        break;
                case 'U':
                        msg.msg_head.opcode = TX_SETUP;
                        msg.msg_head.flags  = 0;
                        break;
                case 'D':
                        msg.msg_head.opcode = TX_DELETE;
                        break;

                default:
                        printf("unknown command '%c'.\n", cmd);
                        exit(1);
                }

                if (!ioctl(sc, SIOCGIFINDEX, &ifr)) {
                        caddr.can_ifindex = ifr.ifr_ifindex;
                        sendto(sc, &msg, sizeof(msg), 0,
                               (struct sockaddr*)&caddr, sizeof(caddr));
                }

        }

        close(sc);
        close(sa);

        return 0;
}

