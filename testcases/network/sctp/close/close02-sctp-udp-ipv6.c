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
 *  FILE        : close02-sctp-udp-ipv6.c
 *  DESCRIPTION  
 *   Test 1: 
 *    Verify that close() of an association would not close other associations. 
 *  
 *   Test 2: 
 *    Verify that on close, a server with multiple associations would send 
 *    each client a shutdown event or a SHUTDOWN_COMPLETE notification. 
 *  
 *  HISTORY:
 *    05/2002  Mingqin Liu 
 *      -Adapted from testcases/kernel/ipc/sem01.c 
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <sys/timeb.h>

#include "test.h"
#include "usctest.h"
#include "../lib/libsctp_test.h"
#include "funutil.h"

#define PORT 10001
#define CLIENT_WAIT_TIME 20 
#define SERVER_WAIT_TIME 3 

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

char *TCID="close02-sctp-udp";	/* Test program identifier. */
int TST_TOTAL = 2; 	/* Total number of test cases. */
extern int Tst_count;

struct sigaction new_act;
int testno;

int semid = 0, shmid1 = 0, shmid2 = 0;
union semun semunion;
struct shmid_ds shminfo;
int *server_ready = (int*) -1, *ready_to_kill = (int*) -1;

int s, ssd, fsd;
pid_t pid;

struct sockaddr_in6 sin1, addr1, addr2, to;

void decode_event(union sctp_notification * sn, int *size, int * got_it); 
void setup(void), setup_server(void);
void cleanup(void),  server_cleanup1(void), server_cleanup2(void);
int semup(int semid);
int semdown(int semid);
void client_alarm_action(), server_alarm_action();
int got_shutdown_event(int sd);


int semup(int semid) {
	struct sembuf semops;
	semops.sem_num = 0;
	semops.sem_op = 1;
	semops.sem_flg = 0;
	if(semop(semid, &semops, 1) == -1) {
		perror("semup");
		return 1;
 	}
	return 0;
}

int semdown(int semid) {
	struct sembuf semops;
	semops.sem_num = 0;
	semops.sem_op = -1;
	semops.sem_flg = 0;
	if(semop(semid, &semops, 1) == -1) {
		perror("semdown: semop");
		return 1;
	}
	return 0;
}

void first_association() {
	int i;

        fsd = socket(PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP);
        if (fsd < 0) {
                tst_brkm(TBROK, cleanup, "socket() setup failed: %s",
                        strerror(errno));
        }

        if (bind(fsd, (struct sockaddr *)&addr1, sizeof(addr1)) < 0) {
                tst_brkm(TBROK, cleanup, "first client bind failed: %s",
                        strerror(errno));
        }
}

void second_association() {
	int i;

        ssd = socket(PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP);
        if (ssd < 0) {
                tst_brkm(TBROK, cleanup, "socket() failed: %s",
                        strerror(errno));
        }

        if (bind(ssd, (struct sockaddr *)&addr2, sizeof(addr2)) < 0) {
                tst_brkm(TBROK, cleanup, "second bind failed: %s",
                        strerror(errno));
        }
}

int main(int argc, char *argv[]) {
	int opt;
	char *msg;
	pid_t pid;
	int chstat;
	int lc;
	int retval;
	struct sctp_event_subscribe event;
	char hello_msg[] = "HELLO! ARE YOU STILL THERE";

        /* Parse standard options given to run the test. */
        msg = parse_opts(argc, argv, (option_t *)NULL, NULL);
        if (msg != (char *)NULL) {
                tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
		return;
        }

        setup();

	pid = fork();
	
	if (pid == 0) {/* child */
		setup_server();
	} else if (pid == -1) {
		tst_brkm(TBROK, cleanup, "fork failed. %s", 
			strerror(errno));
	}
	else { /* parent */

		/* attach to shared memory areas */

		server_ready = (int*) shmat(shmid1, NULL, SHM_RDONLY);
		ready_to_kill = (int*) shmat(shmid2, NULL, 0);

		if ((int *) -1 == server_ready 
		    || (int *) -1 == ready_to_kill) {
                        tst_brkm(TBROK, cleanup, "shmat failed"
                                "test %d", testno);
		}

	        if(semdown(semid)) {
                        tst_brkm(TBROK, cleanup, "semdown failed"
                                "test %d", testno);
		}
		*ready_to_kill = 0;	

		if (!(*server_ready)) {
                        tst_brkm(TBROK, cleanup, "server setup failed"
                                "test %d", testno);
		} 

		/* create two associations. */
		first_association();
		second_association();

		/* issue sendto to set up the connections */
		retval = sendto(ssd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
                	tst_brkm(TBROK, cleanup, "did not get second address"
                        	"test %d", testno);
		} 
	
		retval = sendto(fsd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
                	tst_brkm(TBROK, cleanup, "did not get second address"
                        	"test %d", testno);
		} 

		/* 
		 * close the first association to see whether or not the 
		 * second is still alive.
		 */

		close(fsd);
	
		retval = sendto(ssd, hello_msg, strlen (hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
			tst_resm(TFAIL, "close() of multiple assoc returned %d "
				"(expected 0), errno %d (expected 0)", 
				retval, errno); 
			cleanup();
		} 
		else {
			TEST_ERROR_LOG(0);
			tst_resm(TPASS, "close() of multiple assoc successful");
		}
	

		/*
		 * 2nd test case: 
		 *  Close server socket to see whether the clients get 
		 *  notified of shutdown event.
		 */

		/* start up the first association again */
		first_association();
	
		retval = sendto(fsd, hello_msg, strlen (hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
			tst_resm(TFAIL, "close() of multiple assoc returned %d "
				"(expected 0), errno %d (expected 0)", 
				retval, errno); 
			cleanup();
		} 

		/* turn on shutdown event on both client sockets */

		/*memset(&event, 0, sizeof(event));
		event.sctp_shutdown_event = 1;
		if (setsockopt(fsd, IPPROTO_SCTP, SCTP_SET_EVENT, 
			&event, sizeof(event)) < 0) {
			perror("setsockopt SCTP_SET_EVENT");
		}       

		if (setsockopt(ssd, IPPROTO_SCTP, SCTP_SET_EVENT, 
			&event, sizeof(event)) < 0) {
			perror("setsockopt SCTP_SET_EVENT");
		}*/

		/* issue sendto to set up the connections */
		retval = sendto(ssd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
                	tst_brkm(TBROK, cleanup, "Client sendto failed "
                        	"test %d", testno);
		} 
	
		retval = sendto(fsd, hello_msg, strlen(hello_msg)+1, 0, 
				(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
                	tst_brkm(TBROK, cleanup, "Client sendto failed "
                        	"test %d", testno);
		} 
	 
		/* Tell the server to exit */	
		*ready_to_kill = 1; 

		/* Wait for shutdown event or shutdown_complete notification
		 * on both sockets. 
		 */
		if (got_shutdown_event(fsd) && got_shutdown_event(ssd)) {
			TEST_ERROR_LOG(0);
			tst_resm(TPASS, "got shutdown event as expected");
		} 
		else {
			tst_resm(TFAIL, "close() with multiple assoc "
				"returned %d (expected 0), " 
				"errno %d (expected 0)", retval, errno); 
		}

		cleanup();
	} /* parent */		
}

void
setup(void)
{
	char first_local_addr[128], second_local_addr[128];
	char v6_dev_name[32];
	int got_first_addr = 0, got_second_addr = 0;
        local_addr_t    local_addrs[8];
	int i, count;

        TEST_PAUSE;     /* if -P option specified */ 

	server_ready = NULL;
	ready_to_kill = NULL;
	
	semid = 0;
	shmid1 = 0;
	shmid2 = 0;

        /* get a list of local ip v6 address. */
        get_ip_addresses(local_addrs, &count);

        i = 0;
        got_first_addr = 0;     
        while (i < count && !got_first_addr)
        {
                if ((strcmp(local_addrs[i].if_name, "lo"))
                     && local_addrs[i].has_v6) {
                        strcpy(first_local_addr, local_addrs[i].v6_addr);
                        got_first_addr = 1;
                        strcpy(v6_dev_name, local_addrs[i].if_name);
                } 
                i++;
        } 
        
        i = 0;
        got_second_addr = 0;    

        /* search for another v6 address for the second socket to bind to. */
        while (i < count && !got_second_addr)
        {
                if ((strcmp(local_addrs[i].if_name, "lo"))
                    && (strcmp(local_addrs[i].if_name, v6_dev_name))
                    && local_addrs[i].has_v6) {
                        strcpy(second_local_addr, local_addrs[i].v6_addr);
                        got_second_addr = 1;
                }
                i++;
        }

        if (!got_second_addr) {
                tst_brkm(TBROK, tst_exit, "did not get second address"
                        "test %d", testno);
                tst_exit();
        }

	memset(&addr1, 0, sizeof(addr1));
	addr1.sin6_family = AF_INET;
	inet_pton (AF_INET6, first_local_addr, &addr1.sin6_addr);
	addr1.sin6_port = htons(0);

	memset(&addr2, 0, sizeof(addr2));
	addr2.sin6_family = AF_INET;
	inet_pton (AF_INET6, second_local_addr, &addr2.sin6_addr);
	addr2.sin6_port = htons(0);

	memset(&to, 0, sizeof(to));
	to.sin6_family = AF_INET;
	inet_pton (AF_INET6, first_local_addr, &to.sin6_addr);
	to.sin6_port = htons(PORT);

        /* set up the semaphore and shared memory */
        if((semid = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT)) < 0) {
                tst_brkm(TBROK, tst_exit, "error in semget()");
                tst_exit();
        }

        semunion.val = 0;

        if(semctl(semid, 0, SETVAL, semunion) == -1) {
                tst_brkm(TBROK, cleanup, "error in semctl()");
        }

	if ((shmid1 = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT)) < 0) {
                tst_brkm(TBROK, cleanup, "error in shmget(), %i", errno);
	}

	if ((shmid2 = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT)) < 0) {
                tst_brkm(TBROK, cleanup, "error in shmget(), %i", errno);
	}
	//printf("semid: %i shmid1: %i shmid2: %i\n", semid, shmid1, shmid2);
}


void cleanup (void) {
	close(s);
	close(fsd);
	close(ssd);

	if (server_ready != (int *) -1)
		shmdt(server_ready);
	if (ready_to_kill != (int *)-1)
		shmdt(ready_to_kill);
	(void) wait(NULL);
		
	if (shmid2 > 0 && shmctl(shmid2, IPC_RMID, &shminfo) < 0) {
		printf("error removing shared memory\n");
	}
		
	if (shmid1 > 0 && shmctl(shmid1, IPC_RMID, &shminfo) < 0) {
		printf("error removing shared memory\n");
	}
	
        if (semid > 0 && semctl(semid, 0, IPC_RMID, semunion) == -1) {
        	printf("error removing semaphore\n");
        }
	
	exit(0);
}

void server_cleanup1 (void) {
	if (server_ready != (int *) -1)
		shmdt(server_ready);
	if (ready_to_kill != (int *)-1)
		shmdt(ready_to_kill);

	semup(semid);
	exit(1);
}

void server_cleanup2 (void) {
	close(s);
	close(fsd);
	close(ssd);

	if (server_ready != (int *) -1)
		shmdt(server_ready);
	if (ready_to_kill != (int *)-1)
		shmdt(ready_to_kill);
	exit(0);
}

void setup_server(void)
{
	char buf[1024];
        struct msghdr inmessage;
        char incmsg[CMSG_SPACE(sizeof(sctp_cmsg_data_t))];
        struct cmsghdr *cmsg;
        struct sctp_sndrcvinfo *sinfo;
        struct iovec in_iov;
        char message[1024];
        struct sockaddr_in6 msg_name; 
	int error;
	int previous_flags;

	/* attach the shared memory areas */
	server_ready = (int *) shmat(shmid1, 0, SHM_W|SHM_R);	
	ready_to_kill = (int *) shmat(shmid2, 0, SHM_RDONLY);

	if ((int *)-1 == server_ready  || (int *) -1 ==  ready_to_kill ) {
                tst_brkm(TBROK, server_cleanup1, "server shmat failed: %s",
                        strerror(errno));
        }

	*server_ready = 1; 

        /* initialize sockaddr's */
        sin1.sin6_family = AF_INET;
        sin1.sin6_port = htons(PORT);
        sin1.sin6_addr = (struct in6_addr) IN6ADDR_ANY_INIT;

	s = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	
        if (s < 0) {
		*server_ready = 0;
                tst_brkm(TBROK, server_cleanup1, "socket setup failed: %s",
                        strerror(errno));
        } 
	else if (bind(s, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
		*server_ready = 0;
                tst_brkm(TBROK, server_cleanup1, "server bind failed: %s",
                        strerror(errno));		
        }
        else if (listen(s, 10) < 0) {
		*server_ready = 0;
                tst_brkm(TBROK, server_cleanup1, "server listen failed: %s",
                        strerror(errno));
        }

	/* wake up the parent */
        if(semup(semid)) {
                printf("server: semup failed\n");       
		exit(1);
        }

	if (!(*server_ready)) {
                tst_brkm(TBROK, server_cleanup2, "server setup failed: %s",
                        strerror(errno));
		exit(1);
	}

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

	do {
	        new_act.sa_handler = server_alarm_action;

		sigaction(SIGALRM, &new_act, NULL);


		/*
		 * Interrupt the call within a certain period to check on 
		 * ready_to_kill condition.
		 */
 
		alarm(SERVER_WAIT_TIME);
	        error = recvmsg(s, &inmessage, 0);

		/*printf("server: ");
		if (errno != 4)
			test_print_message(&inmessage, error); */

		/* if it is not an interrupt or error, send the message back 
		 * to sender. 
		 */

                if (errno != 4 && error > 0 
			&& (!(inmessage.msg_flags& MSG_NOTIFICATION))) {
			
			/*printf("Server sendto: %s\n", 
				inmessage.msg_iov[0].iov_base);	 */

                        error = sendto(s, inmessage.msg_iov[0].iov_base,
                                        error, 0, inmessage.msg_name, 
                                        inmessage.msg_namelen);

                } /* if (errno != ...) */


                memset(&inmessage, 0, sizeof(inmessage));

                inmessage.msg_namelen = sizeof (struct sockaddr_in6);
                inmessage.msg_name = (void *)(&msg_name);

                in_iov.iov_len = 1024;
                in_iov.iov_base = message;
                inmessage.msg_iov = &in_iov;
                inmessage.msg_iovlen = 1;

                inmessage.msg_control = incmsg;
                inmessage.msg_controllen = sizeof(incmsg); 
		
		if (errno == 4) {
			error = 0;
		}
        } while (error >= 0 && !(*ready_to_kill)); 
	
	/* if (*ready_to_kill) {
		printf("got killed by parent\n");
	} */

	/* clean up and exit */
	server_cleanup2();

} /* setup_server */

/* 
 * Keep receiving message until we get a shutdown event or a 
 * SHUTDOWN_COMPLETE notification.  
 */
int got_shutdown_event(int sd) {
	struct iovec iov;
        char outcmsg[CMSG_SPACE(sizeof(struct sctp_sndrcvinfo))];
        struct msghdr inmessage;
        int got_it;
        struct sctp_sndrcvinfo *sinfo = NULL;
        struct cmsghdr *scmsg = NULL;
	struct sockaddr_in6 msg_name;
	sctp_cmsg_t type;
	sctp_cmsg_data_t *data;
	char buf[1024];
	
        /* Initialize inmessage with enough space for DATA... */
        memset(&inmessage, 0, sizeof(inmessage));
        iov.iov_base = buf;
        iov.iov_len = 1024;
        inmessage.msg_iov = &iov;
        inmessage.msg_iovlen = 1;
        /* or a control message.  */
        inmessage.msg_control = outcmsg;
        inmessage.msg_controllen = sizeof(outcmsg);
	inmessage.msg_namelen = sizeof(struct sockaddr_in6);
	inmessage.msg_name = (void*)(&msg_name);	
        new_act.sa_handler = client_alarm_action;

	sigaction(SIGALRM, &new_act, NULL);
	alarm(CLIENT_WAIT_TIME);

	got_it = 0;
	while (!got_it) {
		int error;

		//printf("Waiting for shutdown event.\n");
	        error = recvmsg(sd, &inmessage, 0);
	        if (error < 0) { 
			break;	
		}

		/* printf("Client: ");
		test_print_message(&inmessage, error); */

	        for (scmsg = CMSG_FIRSTHDR(&inmessage);
		     scmsg != NULL;
		     scmsg = CMSG_NXTHDR(&inmessage, scmsg)) {
			type = scmsg->cmsg_type;
			data = (sctp_cmsg_data_t *)CMSG_DATA(scmsg);
			/*test_print_cmsg(scmsg->cmsg_type, data); */
		}


		/* if it is a notification, check it out */
		if (MSG_NOTIFICATION & inmessage.msg_flags) {
			int index = 0, body_len;
			char *text;
			int so_far = 0;
			int size = 0;

			text = malloc (error);
			if (NULL == text) {
				printf("Cannot not allocate memory.\n");
				cleanup();	
			}
			body_len = error;

			/* gather the notification body together */
			while (body_len > 0 ) {
				int len;
				len = inmessage.msg_iov[index].iov_len;
                        
				if ( len > body_len ) {
					len = body_len;
				}
				memcpy (text+so_far, 
                                	inmessage.msg_iov[index].iov_base, 
					len);
				so_far += len;
				if ( (body_len -= len) > 0 ) { 
					index++; 
				}
			} /* while (msg_len > 0) */

			body_len = error;
			do {    
				union sctp_notification *sn;

				sn  = (union sctp_notification *)text;
		
				/* On return, got_it will be set to 1 
				 * if it is a shutdown_event or 
				 * SHUTDOWN notification. 
				 */
				decode_event(sn, &size, &got_it);
				body_len -= size;
			} while (body_len > 0 && size != 0 && !got_it);

			free(text);

		} /* if (MSG_NOTIFICATION & smsg->msg_flags) */

	        memset(&inmessage, 0, sizeof(inmessage));
		iov.iov_base = buf;
		iov.iov_len = 1024;
		inmessage.msg_iov = &iov;
		inmessage.msg_iovlen = 1;
		/* or a control message.  */
		inmessage.msg_control = outcmsg;
		inmessage.msg_controllen = sizeof(outcmsg);
		inmessage.msg_namelen = sizeof(struct sockaddr_in6);
		inmessage.msg_name = (void*)(&msg_name);	

	} /* while (!got_it) */

	return got_it;

} /* got_shutdown_event */

/* 
 * server_alarm_action() - reset the alarm.
 *
 */
void server_alarm_action() {

	printf("server waken\n");
        new_act.sa_handler = server_alarm_action;
        sigaction (SIGALRM, &new_act, NULL);
        
        alarm(SERVER_WAIT_TIME);
        return; 
}

/* 
 * client_alarm_action() - reset the alarm.
 *
 */
void client_alarm_action() {

	/* When awaken, kill the server. */
	*ready_to_kill = 1;
        return; 
}

/* 
 * Set got_it to 1 if it is a SHUTDOWN_COMPLETE notification or a shutdown 
 * event. 
 */
 
void decode_event(union sctp_notification * sn, int *size, int * got_it) { 

        if (SCTP_ASSOC_CHANGE == sn->h.sn_type) { 
		if (SHUTDOWN_COMPLETE== sn->sn_assoc_change.sac_state) {
			//printf("Got SHUTDOWN_COMPLETE");
	                *size = sizeof (struct sctp_assoc_change);      
       	        	*got_it = 1; 
		}
        } 
        else if (SCTP_PEER_ADDR_CHANGE== sn->h.sn_type) {
                *size = sizeof (struct sctp_paddr_change);
        }  
        else if (SCTP_REMOTE_ERROR== sn->h.sn_type) {
                *size = sizeof (struct sctp_remote_error);
        } 
        else if (SCTP_SEND_FAILED== sn->h.sn_type) {
                *size = sizeof (struct sctp_send_failed);
        }  
        else if (SCTP_SHUTDOWN_EVENT== sn->h.sn_type) {
                *size = sizeof (struct sctp_shutdown_event);
               	*got_it = 1; 
        } 
        /*else if (SCTP_ADAPTION_INDICATION== sn->h.sn_type) {
                *size = sizeof (struct sctp_adaption_indication;
        }
        else if (SCTP_PARTIAL_DELIVERY_EVENT== sn->h.sn_type) {
                *size = sizeof (struct sctp_partial_delivery_event);
        }*/
} /* decode_event */

