/*
 *  $Id: tst-proc.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-proc.c
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
#include <linux/can/raw.h>

#define MAX_RAW 800

int main(int argc, char **argv)
{
        int s[MAX_RAW];
        struct sockaddr_can addr;
        struct ifreq ifr;
        int i,numsock;

        if (argc != 2) {
                fprintf(stderr, "Error: Wrong number of arguments. Try %s <number of created sockets>.\n", argv[0]);
                exit(1);
        }

        numsock = atoi(argv[1]);

        if (numsock >= MAX_RAW) {
                fprintf(stderr, "Error: more than %d sockets to open (see #define MAX_RAW).\n", MAX_RAW);
                exit(1);
        }

        printf("\ncreating %d raw sockets ... ", numsock);

        if (numsock) {
                for (i=0; i < numsock; i++) {
                        if ((s[i] = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
                                perror("socket");
                                return 1;
                        }

                        addr.can_family = PF_CAN;
                        strcpy(ifr.ifr_name, "vcan2");
                        ioctl(s[i], SIOCGIFINDEX, &ifr);
                        addr.can_ifindex = ifr.ifr_ifindex;

                        if (bind(s[i], (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                                perror("connect");
                                return 1;
                        }
                }
        }

        printf("done.\n");

        printf("Waiting for keyboard input ...");

        getchar();

        printf("closing %d raw sockets ... ", numsock);

        if (numsock)
                for (i=0; i < numsock; i++)
                        close(s[i]);

        printf("done.\n\n");

        return 0;

}

