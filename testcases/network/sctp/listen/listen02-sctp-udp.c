/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Test Name: listen02-sctp-udp
 *
 * Test Description:
 *      Check to see whether a listen before a bind/bindx triggers 
 *	ephemeral port and autobind features. 
 *    
 * ALGORITHM
 *
 * Usage:  <for command-line>
 *  listen02-sctp-udp [-c n] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	05/2002 Adapted for SCTP by Mingqin Liu
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <netinet/sctp.h>

#include "test.h"
#include "usctest.h"
#include "funutil.h"
#include "../lib/libsctp_test.h"

#define PORT 10001
#define RUN_TIME 10
#define HELLO_MSG "HELLO! ARE YOU STILL THERE"


char *TCID="listen01-sctp-udp";		/* Test program identifier.    */
int testno;
int TST_TOTAL=1; /* Total number of test cases. */
extern int Tst_count;

struct sigaction new_act;
int testno;
int verbose = 0;
int loops = 100;

int s, fsd;
pid_t pid;

int msg_sent, ready_to_kill;

struct sockaddr_in sin1, addr1, to;

void setup(void);
pid_t setup_server(void);
void cleanup(void);
void alarm_action();

void first_association() {
	int i;

        fsd = socket(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
        if (fsd < 0) {
                tst_brkm(TBROK, cleanup, "socket setup failed: %s",
                        strerror(errno));
		return;
        }

        if (bind(fsd, (struct sockaddr *)&addr1, sizeof(addr1)) < 0) {
                tst_brkm(TBROK, cleanup, "second bind failed: %s",
                        strerror(errno));
                return;
        }
}

int
main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t pid;
	int retval;
	
	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, (option_t *) NULL, NULL);
	if (msg != (char *) NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	msg_sent = 0;
        setup();
	pid = fork();
	
	if (pid == 0) {/* child */
		setup_server();
	} else if (pid == -1) {
		tst_brkm(TBROK, cleanup, "server fork failed. %s", 
			strerror(errno));
		/* fall through */	
	}
	else { /* parent */
		close(s);
		first_association();
	
		retval = sendto(fsd, HELLO_MSG, strlen (HELLO_MSG)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
                	tst_brkm(TBROK, cleanup, "close on shared socket "
                        	"test %d", testno);
	                tst_exit();
		} 
	
		msg_sent = 1;

		sleep(2*RUN_TIME);
		close(fsd);
		ready_to_kill = 1;

	} /* parent */		
}

void
setup(void)
{
	char first_local_addr[128], second_local_addr[128];
	char v4_dev_name[32];
	int got_first_addr = 0, got_second_addr = 0;
        local_addr_t    local_addrs[10];
	int i, count;

        TEST_PAUSE;     /* if -P option specified */ 

        /* get a list of ip v4 address. */
        get_ip_addresses(local_addrs, &count);

        i = 0;
        got_first_addr = 0;     
        while (i < count && !got_first_addr)
        {
                if ((strcmp(local_addrs[i].if_name, "lo"))
                     && local_addrs[i].has_v4) {
                        strcpy(first_local_addr, local_addrs[i].v4_addr);
                        got_first_addr = 1;
                        strcpy(v4_dev_name, local_addrs[i].if_name);
                } 
                i++;
        } 
        
        i = 0;
        got_second_addr = 0;    

        /* search for another different v4 address. */
        while (i < count && !got_second_addr)
        {
                /*printf("if_name: %s dev: %s has_v4: %i\n", 
                        local_addrs[i].if_name, v4_dev_name, 
                        local_addrs[i].has_v4); */
                if ((strcmp(local_addrs[i].if_name, "lo"))
                    && (strcmp(local_addrs[i].if_name, v4_dev_name))
                    && local_addrs[i].has_v4) {
                        strcpy(second_local_addr, local_addrs[i].v4_addr);
                        got_second_addr = 1;
                }
                i++;
        }

       /* if (!got_second_addr) {
                tst_brkm(TBROK, cleanup, "did not get second address"
                        "test %d", testno);
                tst_exit();
        } */

	memset(&addr1, 0, sizeof(addr1));
	addr1.sin_family = AF_INET;
	addr1.sin_addr.s_addr = inet_addr(first_local_addr);
	addr1.sin_port = htons(0);

	memset(&to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = inet_addr(first_local_addr);
	to.sin_port = htons(PORT);
}

void cleanup (void) {
	close(s);
	close(fsd);
}

pid_t setup_server(void)
{
	char buf[1024];
	struct sockaddr_in from;
        struct msghdr inmessage;
        char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
        struct cmsghdr *cmsg;
        struct sctp_sndrcvinfo *sinfo;
        struct iovec in_iov;
        char message[1024];
        struct sockaddr_in6 msg_name; 
	int error;
	int previous_flags;
	int sinlen;
	struct sockaddr_in server;

	sinlen = sizeof(struct sockaddr_in);

	memset(&server, 0, sizeof(server));
        /* initialize sockaddr's */
        sin1.sin_family = AF_INET;
        sin1.sin_port = htons(PORT);
        sin1.sin_addr.s_addr = INADDR_ANY;

        s = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
        if (s < 0) {
                tst_brkm(TBROK, cleanup, "socket setup failed: %s",
                        strerror(errno));
        }

        if (bind(s, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
                tst_brkm(TBROK, cleanup, "server bind failed: %s",
                        strerror(errno));
                return;
        } 
        if (listen(s, 10) < 0) {
                tst_brkm(TBROK, cleanup, "server listen failed: %s",
                        strerror(errno));
                return;
        }
	server.sin_port = htons(16);
	if (getsockname(s, (struct sockaddr *) &server, &sinlen)<0) {
		printf("error getsockname\n");
		return;
	}
	if (sinlen != sizeof(server)) {
		printf("getsockname returned unexpected value\n");
		return;
	}
	printf("returned len: %i\n", sinlen);
	printf("Server has port %hu\n", ntohs(server.sin_port));

        memset(&inmessage, 0, sizeof(inmessage));

        inmessage.msg_namelen = sizeof (struct sockaddr_in6);
        inmessage.msg_name = (void *)(&msg_name);

        in_iov.iov_len = 1024;
        in_iov.iov_base = message;

        inmessage.msg_iov = &in_iov;
        inmessage.msg_iovlen = 1;

        /* or a control message.  */
        inmessage.msg_control = incmsg;
        inmessage.msg_controllen = sizeof(incmsg); 


        new_act.sa_handler = alarm_action;

        sigaction(SIGALRM, &new_act, NULL);
        alarm(RUN_TIME);

        error = recvmsg(s, &inmessage, 0);
	
        printf("server: ");
        test_print_message(&inmessage, error);

	if (error <=0) {
		printf("failed\n");
		exit(1);
	}
	previous_flags = inmessage.msg_flags;

        do {
                if (error > 0 && (!(previous_flags & MSG_NOTIFICATION))) {
			printf("got data\n");
			break;
                } /* if (error > 0) */
		else 
			printf("NOTe\n");

        	alarm(RUN_TIME);
                error = recvmsg(s, &inmessage, 0);
        	printf("server: ");
	        test_print_message(&inmessage, error);

		previous_flags = inmessage.msg_flags;
                memset(&inmessage, 0, sizeof(inmessage));

                inmessage.msg_namelen = sizeof (struct sockaddr_in6);
                inmessage.msg_name = (void *)(&msg_name);

                in_iov.iov_len = 1024;
                in_iov.iov_base = message;
                inmessage.msg_iov = &in_iov;
                inmessage.msg_iovlen = 1;

                inmessage.msg_control = incmsg;
                inmessage.msg_controllen = sizeof(incmsg); 

        } while (error >= 0 && !ready_to_kill); 
	exit(0);

} /* setup_server */


/* 
 * alarm_action() - reset the alarm.
 *
 */
void alarm_action() {

        new_act.sa_handler = alarm_action;
        sigaction (SIGALRM, &new_act, NULL);
        
        alarm(RUN_TIME);
        return; 
}
