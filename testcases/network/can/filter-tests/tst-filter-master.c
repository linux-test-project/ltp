/*
 *  $Id: tst-filter-master.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-filter-master.c
 *
 * Copyright (c) 2008 Volkswagen Group Electronic Research
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
        struct can_filter rfilter;
        struct can_frame frame;
        int testcase;
        int nbytes;
        struct ifreq ifr;
        int ifindex;


        if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
                perror("socket");
                return 1;
        }

        strcpy(ifr.ifr_name, "vcan0");
        ioctl(s, SIOCGIFINDEX, &ifr);
        ifindex = ifr.ifr_ifindex;

        addr.can_family = AF_CAN;
        addr.can_ifindex = ifindex;

        rfilter.can_id   = 0xFA; /* receive only the filter ack */
        rfilter.can_mask = CAN_SFF_MASK | CAN_EFF_FLAG | CAN_RTR_FLAG;
        setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("bind");
                return 1;
        }

        /* send testcases 0 .. 17 and a terminating 18 to quit */
        for (testcase = 0; testcase < 19; testcase++) {

                printf("Sending testcase %2d ... ", testcase);
                frame.can_id = 0x0F;
                frame.can_dlc = 1;
                frame.data[0] = testcase;

                if (write(s, &frame, sizeof(frame)) < 0) {
                        perror("write");
                        exit(1);
                }

                /* wait for ACK from server */
                if ((nbytes = read(s, &frame, sizeof(struct can_frame))) < 0) {
                        perror("read");
                        exit(1);
                }

                if (nbytes < sizeof(struct can_frame)) {
                        fprintf(stderr, "read: incomplete CAN frame\n");
                        exit(1);
                }

                if ((frame.can_id != 0xFA) || (frame.can_dlc != 1) ||
                    (frame.data[0] != testcase)) {
                        fprintf(stderr, "\nWrong answer from server!\n");
                        exit(1);
                }

                printf("acked. ");
                if (testcase > 17)
                        break;

                /* interactive mode, when there is any commandline option */
                if (argc == 2) {
                        printf("[press enter] ");
                        getchar();
                }

                printf("Sending patterns ... ");

                frame.can_dlc = 0;

                frame.can_id = 0x123;
                if (write(s, &frame, sizeof(frame)) < 0) {
                        perror("write");
                        exit(1);
                }
                frame.can_id = (0x123 | CAN_RTR_FLAG);
                if (write(s, &frame, sizeof(frame)) < 0) {
                        perror("write");
                        exit(1);
                }
                frame.can_id = (0x123 | CAN_EFF_FLAG);
                if (write(s, &frame, sizeof(frame)) < 0) {
                        perror("write");
                        exit(1);
                }
                frame.can_id = (0x123 | CAN_EFF_FLAG | CAN_RTR_FLAG);
                if (write(s, &frame, sizeof(frame)) < 0) {
                        perror("write");
                        exit(1);
                }

                printf("ok\n");
        }

        printf("Filtertest done.\n");

        close(s);
        return 0;
}

