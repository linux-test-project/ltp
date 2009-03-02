/*
 *  $Id: tst-filter-server.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-filter-server.c
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

#define ID  0x123
#define FIL 0x7FF
#define EFF CAN_EFF_FLAG
#define RTR CAN_RTR_FLAG

canid_t calc_id(int testcase)
{
        canid_t id = ID;

        if (testcase & 1)
                id |= EFF;
        if (testcase & 2)
                id |= RTR;

        return id;
}

canid_t calc_mask(int testcase)
{
        canid_t mask = FIL;

        if (testcase > 15)
                return (CAN_EFF_MASK | CAN_EFF_FLAG | CAN_RTR_FLAG);

        if (testcase & 4)
                mask |= EFF;
        if (testcase & 8)
                mask |= RTR;

        return mask;
}

int main(int argc, char **argv)
{
        fd_set rdfs;
        int s, t;
        struct sockaddr_can addr;
        struct can_filter rfilter;
        struct can_frame frame;
        int testcase = 0;
        int nbytes, ret;
        struct ifreq ifr;
        int ifindex;


        if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
                perror("socket");
                return 1;
        }
        if ((t = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
                perror("socket");
                return 1;
        }

        strcpy(ifr.ifr_name, "vcan0");
        ioctl(s, SIOCGIFINDEX, &ifr);
        ifindex = ifr.ifr_ifindex;

        addr.can_family = AF_CAN;
        addr.can_ifindex = ifindex;

        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("bind");
                return 1;
        }
        if (bind(t, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("bind");
                return 1;
        }

        rfilter.can_id   = 0xF; /* receive only the filter requests */
        rfilter.can_mask = CAN_SFF_MASK | CAN_EFF_FLAG | CAN_RTR_FLAG;
        setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

        /* disable default receive filter on the test socket */
        setsockopt(t, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

        while (1) {
                
                FD_ZERO(&rdfs);
                FD_SET(s, &rdfs);
                FD_SET(t, &rdfs);

                if ((ret = select(t+1, &rdfs, NULL, NULL, NULL)) < 0) {
                        perror("select");
                        break;
                }

                if (FD_ISSET(s, &rdfs)) {

                        if ((nbytes = read(s, &frame, sizeof(struct can_frame))) < 0) {
                                perror("read");
                                exit(1);
                        }

                        if (nbytes < sizeof(struct can_frame)) {
                                fprintf(stderr, "read: incomplete CAN frame\n");
                                exit(1);
                        }

                        if ((frame.can_id != 0xF) || (frame.can_dlc != 1)) {
                                fprintf(stderr, "\nWrong request from master!\n");
                                exit(1);
                        }

                        testcase = frame.data[0];

                        if (testcase < 18) {
                                rfilter.can_id   = calc_id(testcase);
                                rfilter.can_mask = calc_mask(testcase);
                                setsockopt(t, SOL_CAN_RAW, CAN_RAW_FILTER,
                                           &rfilter, sizeof(rfilter));

                                printf("testcase %2d : can_id = 0x%08X can_mask = 0x%08X\n",
                                       testcase, rfilter.can_id, rfilter.can_mask);
                        }

                        frame.can_id = 0xFA; /* filter ack */

                        if (write(s, &frame, sizeof(frame)) < 0) {
                                perror("write");
                                exit(1);
                        }

                        if (testcase > 17)
                                break;
                }

                if (FD_ISSET(t, &rdfs)) {

                        if ((nbytes = read(t, &frame, sizeof(struct can_frame))) < 0) {
                                perror("read");
                                exit(1);
                        }

                        if (nbytes < sizeof(struct can_frame)) {
                                fprintf(stderr, "read: incomplete CAN frame\n");
                                exit(1);
                        }
                        
                        printf ("%08X\n", frame.can_id);
                }
        }

        printf("testcase %2d : Filtertest done.\n", testcase);

        close(s);
        close(t);

        return 0;
}

