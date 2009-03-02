/*
 *  $Id: canecho.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * canecho.c
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
#include <signal.h>
#include <libgen.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>

#include <linux/can.h>

extern int optind, opterr, optopt;

static int      s = -1;
static int      running = 1;

void print_usage(char *prg)
{
        fprintf(stderr, "Usage: %s [can-interface]\n", prg);
}

void sigterm(int signo)
{
        printf("got signal %d\n", signo);
        running = 0;
}

int main(int argc, char **argv)
{
        int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;
        int opt;
        struct sockaddr_can addr;
        struct ifreq ifr;
        struct can_frame frame;
        int nbytes, i;
        int verbose = 0;

        signal(SIGTERM, sigterm);
        signal(SIGHUP, sigterm);

        while ((opt = getopt(argc, argv, "f:t:p:v")) != -1) {
                switch (opt) {
                case 'f':
                        family = atoi(optarg);
                        break;

                case 't':
                        type = atoi(optarg);
                        break;

                case 'p':
                        proto = atoi(optarg);
                        break;

                case 'v':
                        verbose = 1;
                        break;

                case '?':
                        break;

                default:
                        fprintf(stderr, "Unknown option %c\n", opt);
                        break;
                }
        }

        if (optind == argc) {
                print_usage(basename(argv[0]));
                exit(0);
        }
        
        printf("interface = %s, family = %d, type = %d, proto = %d\n",
               argv[optind], family, type, proto);
        if ((s = socket(family, type, proto)) < 0) {
                perror("socket");
                return 1;
        }

        addr.can_family = family;
        strcpy(ifr.ifr_name, argv[optind]);
        ioctl(s, SIOCGIFINDEX, &ifr);
        addr.can_ifindex = ifr.ifr_ifindex;

        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("bind");
                return 1;
        }

        while (running) {
                if ((nbytes = read(s, &frame, sizeof(frame))) < 0) {
                        perror("read");
                        return 1;
                }
                if (verbose) {
                        printf("%03X: ", frame.can_id & CAN_EFF_MASK);
                        if (frame.can_id & CAN_RTR_FLAG) {
                                printf("remote request");
                        } else {
                                printf("[%d]", frame.can_dlc);
                                for (i = 0; i < frame.can_dlc; i++) {
                                        printf(" %02X", frame.data[i]);
                                }
                        }
                        printf("\n");
                }
                frame.can_id++;
                write(s, &frame, sizeof(frame));
        }

        return 0;
}

