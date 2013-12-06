/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 *
 * This file does send and receive for 500 threads on a unique association for
 * THREAD_SND_RCV_LOOPS = 10 many times. To change the number of threads 
 * change the THREADS valuen and loop change the THREAD_SND_RCV_LOOPS.
 *
 * The SCTP implementation is free software;
 * you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * The SCTP implementation is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 ************************
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU CC; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Please send any bug reports or fixes you make to the
 * email address(es):
 *    lksctp developers <lksctp-developers@lists.sourceforge.net>
 *
 * Or submit a bug report through the following website:
 *    http://www.sf.net/projects/lksctp
 *
 * Any bugs reported given to us we will try to fix... any fixes shared will
 * be incorporated into the next SCTP release
 *
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>         /* for sockaddr_in */
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/sctp.h>
#include <sys/uio.h>
#include <linux/socket.h>
#include <sctputil.h>

#define THREADS 10    /* FIXME should be 500 instead of 10 */
#define THREAD_SND_RCV_LOOPS 10

char *TCID = __FILE__;
int TST_TOTAL = 1;
int TST_CNT = 0;

int client_sk;
int server_sk;
int acpt_sk;
struct sockaddr_in  conn_addr;
char *message = "hello, world!\n";

void 
t_recv(void) {
	int cnt;
	struct msghdr inmessage;
	struct iovec iov;
        char incmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
        char * buffer;

	memset(&inmessage, 0, sizeof(inmessage));
        buffer = malloc(100);

        iov.iov_base = buffer;
        iov.iov_len = 100;
        inmessage.msg_iov = &iov;
        inmessage.msg_iovlen = 1;
        inmessage.msg_control = incmsg;
        inmessage.msg_controllen = sizeof(incmsg);

	cnt = test_recvmsg(acpt_sk,&inmessage, MSG_WAITALL);
        test_check_msg_data(&inmessage, cnt, strlen(message) + 1, MSG_EOR,
			    0, 0);
}

void
t_send(void) {
        struct msghdr outmessage;
        struct sctp_sndrcvinfo *sinfo;
        struct cmsghdr *cmsg;
        struct iovec out_iov;
        char outcmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];

        memset(&outmessage, 0, sizeof(outmessage));
        outmessage.msg_name = &conn_addr;
        outmessage.msg_namelen = sizeof(conn_addr);
        outmessage.msg_iov = &out_iov;
        outmessage.msg_iovlen = 1;
        outmessage.msg_control = outcmsg;
        outmessage.msg_controllen = sizeof(outcmsg);
        outmessage.msg_flags = 0;

        cmsg = CMSG_FIRSTHDR(&outmessage);
	cmsg->cmsg_level = IPPROTO_SCTP;
        cmsg->cmsg_type = SCTP_SNDRCV;
        cmsg->cmsg_len = CMSG_LEN(sizeof(struct sctp_sndrcvinfo));
        outmessage.msg_controllen = cmsg->cmsg_len;

        sinfo = (struct sctp_sndrcvinfo *)CMSG_DATA(cmsg);
        memset(sinfo, 0x00, sizeof(struct sctp_sndrcvinfo));
        outmessage.msg_iov->iov_base = message;
        outmessage.msg_iov->iov_len = (strlen(message) + 1);

        test_sendmsg(client_sk, &outmessage, 0, strlen(message)+1);
}

void *relay(void *arg)
{
	int id = *(int *) arg;

	if (id == 0) {
		t_send();
	} else {
		t_recv();
		t_send();
	}

	pthread_exit(NULL);
}

int 
main(void) 
{

	int      cnt,i;
	int      pth[THREADS];
	pthread_t       thread[THREADS];
	int  status;
	int  exit_status;
	void *      result;
	pthread_attr_t attr;
	struct sockaddr_in lstn_addr;
	socklen_t len = sizeof(struct sockaddr_in);
	struct sockaddr_in svr_addr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	server_sk = test_socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP);
	client_sk = test_socket(PF_INET, SOCK_STREAM, IPPROTO_SCTP);

	lstn_addr.sin_family = AF_INET;
        lstn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	lstn_addr.sin_port = htons(SCTP_TESTPORT_1);

	conn_addr.sin_family = AF_INET;
        conn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	conn_addr.sin_port = htons(SCTP_TESTPORT_1);

	test_bind(server_sk, (struct sockaddr *)&lstn_addr,
		 sizeof(struct sockaddr_in));

	test_listen(server_sk,10);

	test_connect(client_sk,(struct sockaddr *)&conn_addr,len);

	acpt_sk = test_accept(server_sk, (struct sockaddr *)&svr_addr, &len);

	for ( i = 0; i < THREAD_SND_RCV_LOOPS; i++ ) {
		for (cnt = 0; cnt < THREADS; cnt++) {
			pth[cnt] = cnt;
			status = pthread_create(&thread[cnt], &attr, relay, &pth[cnt]);
			if (status)
				tst_brkm(TBROK, tst_exit, "pthread_create "
                         		 "failed status:%d, errno:%d", status,
					 errno);
		}

		pthread_attr_destroy(&attr);
		for (cnt = 0; cnt < THREADS ; cnt++) {
			exit_status = pthread_join (thread[cnt], &result);
			if (exit_status == -1)
				tst_brkm(TBROK, tst_exit, "pthread_join "
                         		 "Thread #%d exited with status:%d",
					 cnt, exit_status);
		}
	}

	tst_resm(TPASS, "send and receive data across multiple threads - "
		 "SUCCESS");

	return 0;
}
