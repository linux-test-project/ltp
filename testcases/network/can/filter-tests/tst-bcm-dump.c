/*
 *  $Id: tst-bcm-dump.c,v 1.1 2009/03/02 15:33:55 subrata_modak Exp $
 */

/*
 * tst-bcm-dump.c
 *
 * Copyright (c) 2008 Oliver Hartkopp
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Send feedback to <socketcan-users@lists.berlios.de>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <libgen.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/bcm.h>

#define DEFAULT_IFACE "vcan0"
#define DEFAULT_CANID 0x42

void print_usage(char *prg)
{
        fprintf(stderr, "\nUsage: %s [options]\n", prg);
        fprintf(stderr, "Options: -i <interface> (CAN interface. Default: '%s')\n", DEFAULT_IFACE);
        fprintf(stderr, "         -c <can_id>    (used CAN ID. Default: 0x%03X)\n", DEFAULT_CANID);
        fprintf(stderr, "         -o <timeout>   (Timeout value in nsecs. Default: 0)\n");
        fprintf(stderr, "         -t <throttle>  (Throttle value in nsecs. Default: 0)\n");
        fprintf(stderr, "         -q <msgs>      (Quit after receiption of #msgs)\n");
        fprintf(stderr, "         -s             (set STARTTIMER flag. Default: off)\n");
        fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
        int s;
        struct sockaddr_can addr;
        int nbytes;
        int i;
        struct ifreq ifr;
        char *ifname = DEFAULT_IFACE;
        canid_t canid = DEFAULT_CANID;
        int opt;
        struct timeval tv;
        unsigned long starttimer = 0;
        unsigned long long timeout = 0;
        unsigned long long throttle = 0;
        unsigned long msgs = 0;
        struct {
                struct bcm_msg_head msg_head;
                struct can_frame frame;
        } msg;

        while ((opt = getopt(argc, argv, "i:c:o:t:q:s")) != -1) {
                switch (opt) {

                case 'i':
                        ifname = optarg;
                        break;

                case 'c':
                        canid = strtoul(optarg, (char **)NULL, 16);
                        break;

                case 'o':
                        timeout = strtoull(optarg, (char **)NULL, 10);
                        break;

                case 't':
                        throttle = strtoull(optarg, (char **)NULL, 10);
                        break;

                case 'q':
                        msgs = strtoul(optarg, (char **)NULL, 10);
                        break;

                case 's':
                        starttimer = STARTTIMER;
                        break;

                case '?':
                default:
                        print_usage(basename(argv[0]));
                        exit(1);
                        break;
                }
        }

        if ((s = socket(PF_CAN, SOCK_DGRAM, CAN_BCM)) < 0) {
                perror("socket");
                return 1;
        }

        if (strcmp(ifname, "any") == 0)
                addr.can_ifindex = 0;
        else {
                strcpy(ifr.ifr_name, ifname);
                if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
                        perror("SIOCGIFINDEX");
                        return 1;
                }
                addr.can_ifindex = ifr.ifr_ifindex;
        }

        addr.can_family = PF_CAN;

        if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("connect");
                return 1;
        }

        msg.msg_head.opcode             = RX_SETUP;
        msg.msg_head.can_id             = canid;
        msg.msg_head.flags              = SETTIMER|RX_FILTER_ID|starttimer;
        msg.msg_head.ival1.tv_sec       = timeout / 1000000;
        msg.msg_head.ival1.tv_usec      = timeout % 1000000;
        msg.msg_head.ival2.tv_sec       = throttle / 1000000;
        msg.msg_head.ival2.tv_usec      = throttle % 1000000;
        msg.msg_head.nframes    = 0;

        gettimeofday(&tv, NULL);
        printf("[%ld.%06ld] ", tv.tv_sec, tv.tv_usec);
        printf("Writing RX_SETUP with RX_FILTER_ID for can_id <%03X>\n",
               msg.msg_head.can_id);

        if (write(s, &msg, sizeof(msg)) < 0)
                perror("write");

        while (1) {

                nbytes = read(s, &msg, sizeof(msg));
                if (nbytes < 0) {
                        perror("read");
                        return 1;
                }
                gettimeofday(&tv, NULL);
                printf("[%ld.%06ld] ", tv.tv_sec, tv.tv_usec);

                if (nbytes == sizeof(msg)) {

                        if (ioctl(s, SIOCGSTAMP, &tv) < 0)
                                perror("SIOCGSTAMP");
                        else
                                printf("(%ld.%06ld) ", tv.tv_sec, tv.tv_usec);

                        if (msg.msg_head.opcode != RX_CHANGED) {
                                printf("missing RX_CHANGED.\n");
                                return 1;
                        }

                        printf("RX_CHANGED ");

                        for (i=0; i < msg.frame.can_dlc; i++)
                                printf("%02X ", msg.frame.data[i]);

                } else {

                        if (msg.msg_head.opcode != RX_TIMEOUT) {
                                printf("missing RX_TIMEOUT.\n");
                                return 1;
                        }

                        printf("RX_TIMEOUT");
                }

                printf("\n");
                fflush(stdout);

                if (msgs && !(--msgs))
                        break;
        }

        close(s);

    return 0;
}