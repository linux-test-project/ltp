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
 *  FILE        : close05-sctp-udp.c
 *  DESCRIPTION 
  *   Test 1: 
 *    Verify that close() of the same socket descriptor in different 
 *    processes would not interfere with each other. 
 *
 *   Test 2:      
 *    Verify that a second close() of the same socket desriptors returns 
 *    error.
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

#include "test.h"
#include "usctest.h"
#include "funutil.h"
#include "../lib/libsctp_test.h"

#define PORT 10001
#define RUN_TIME 10 
#define HELLO_MSG "HELLO! ARE YOU STILL THERE"

union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
};


char *TCID="close05-sctp-udp";	/* Test program identifier. */
int TST_TOTAL = 2; 	/* Total number of test cases. */
extern int Tst_count;

struct sigaction new_act;
int testno;
int verbose = 0;
int loops = 100;

int s, fsd;
pid_t pid;

int semid, shmid1, shmid2;
union semun semunion;
struct shmid_ds shminfo;
int *server_ready, *ready_to_kill;

struct sockaddr_in6 sin1, addr1, to;

void decode_event(union sctp_notification * sn, int *size, int * got_it);
void setup(void), setup_server(void);
void cleanup(void), server_cleanup1(void), server_cleanup2(void);
int semup(int semid), semdown(int semid);
void client_alarm_action(), server_alarm_action();

int semup(int semid) {
        struct sembuf semops;
        semops.sem_num = 0;
        semops.sem_op = 1;
        semops.sem_flg = 0;
        printf("up, semid: %i\n", semid);
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
        printf("down, semid: %i\n", semid);
        if(semop(semid, &semops, 1) == -1) {
                perror("semdown: semop");
                return 1;
        }
        return 0;
}

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

int main(int argc, char *argv[]) {
	int opt;
	char *msg;
	pid_t pid;
	int lc;
	int retval;

        /* Parse standard options given to run the test. */
        msg = parse_opts(argc, argv, (option_t *)NULL, NULL);
        if (msg != (char *)NULL) {
                tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
		return;
        }

        setup();

	first_association();
		
	pid = fork();
	
	if (pid == 0) {/* child */
		setup_server();
	} else if (pid == -1) {
		tst_brkm(TBROK, cleanup, "server fork failed. %s", 
			strerror(errno));
		/* fall through */	
	}
	else { /* parent */
                server_ready = (int*) shmat(shmid1, NULL, SHM_RDONLY);
                ready_to_kill = (int*) shmat(shmid2, NULL, 0);

                if ((int *) -1 == server_ready
                    || (int *) -1 == ready_to_kill) {
                	tst_brkm(TBROK, cleanup, "shmat failed"
                        	"test %d", testno);
                        cleanup();
                }

                if(semdown(semid)) {
                	tst_brkm(TBROK, cleanup, "semdown failed"
                        	"test %d", testno);
                }
                *ready_to_kill = 0;

                if (!(*server_ready)) {
                        printf("SERver is NOT READY\n");
                        cleanup();
                } else {
	                printf("SERVER IS READY\n");
                }
	
		retval = sendto(fsd, HELLO_MSG, strlen (HELLO_MSG)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
                	tst_brkm(TBROK, cleanup, "close on shared socket "
                        	"test %d", testno);
		} 
	
		/*
		 * 2nd test
		 *  Close the same socket twice to see whether the second 
		 *  fails.  
		 */
		retval = sendto(fsd, HELLO_MSG, strlen (HELLO_MSG)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
			tst_resm(TFAIL, "close() on shared socket failed. "					" returned %d "
				"(expected -1), errno %d (expected 0)", 
				retval, errno); 
		} else {
			TEST_ERROR_LOG(0);
			tst_resm(TPASS, "close() on shared socket successful");

		}


		/* 
		 * close the socket twice to see whether or not the 
		 * second all returns error as expected. 
		 */

		close(fsd);

		if (close(fsd) != -1) {
			tst_resm(TFAIL, "multiple close() returned %d "
				"(expected -1), errno %d (expected 0)", 
				retval, errno); 
		} else {
			TEST_ERROR_LOG(0);
			tst_resm(TPASS, "multiple close() successful");
		}
	
		*ready_to_kill = 1;
		
		cleanup();

	} /* parent */		
}

void
setup(void)
{
	char first_local_addr[128], second_local_addr[128];
	char v6_dev_name[32];
	int got_first_addr = 0, got_second_addr = 0;
        local_addr_t    local_addrs[10];
	int i, count;

        TEST_PAUSE;     /* if -P option specified */ 

        server_ready = NULL;
        ready_to_kill = NULL;

        semid = 0;
        shmid1 = 0;
        shmid2 = 0;

        /* get a list of ip v6 address. */
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

        /* search for another different v666666 address. */
        while (i < count && !got_second_addr)
        {
                /*printf("if_name: %s dev: %s has_v6: %i\n", 
                        local_addrs[i].if_name, v6_dev_name, 
                        local_addrs[i].has_v6); */
                if ((strcmp(local_addrs[i].if_name, "lo"))
                    && (strcmp(local_addrs[i].if_name, v6_dev_name))
                    && local_addrs[i].has_v6) {
                        strcpy(second_local_addr, local_addrs[i].v6_addr);
                        got_second_addr = 1;
                }
                i++;
        }

        if (!got_second_addr) {
                tst_brkm(TBROK, cleanup, "did not get second address"
                        "test %d", testno);
                tst_exit();
        }

	memset(&addr1, 0, sizeof(addr1));
        addr1.sin6_family = AF_INET;
        addr1.sin6_port = htons(0);
        if (!inet_pton(AF_INET6, first_local_addr, &(addr1.sin6_addr))) {
                tst_brkm(TBROK, tst_exit, "invalid addr: %s test %d", 
                first_local_addr, testno);
        }

	memset(&to, 0, sizeof(to));
        to.sin6_family = AF_INET;
        to.sin6_port = htons(PORT);

        if (!inet_pton(AF_INET6, first_local_addr, &(to.sin6_addr))) {
                tst_brkm(TBROK, tst_exit, "invalid addr: %s test %d", 
                first_local_addr, testno);
        }

        /* set up the semaphore and shared memory */
        if((semid = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT)) < 0) {
                tst_brkm(TBROK, tst_exit, "error in semget()");
                tst_exit();
        }
        printf("semid: %i shmid1: %i shmid2: %i\n", semid, shmid1, shmid2);
 
        semunion.val = 0;

        if(semctl(semid, 0, SETVAL, semunion) == -1) {
                tst_brkm(TBROK, cleanup, "error in semctl()");
                tst_exit(1);
        }
        printf("semid: %i shmid1: %i shmid2: %i\n", semid, shmid1, shmid2);

        if ((shmid1 = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT)) < 0) {
                tst_brkm(TBROK, cleanup, "error in shmget(), %i", errno);
                tst_exit();     
        }
        printf("semid: %i shmid1: %i shmid2: %i\n", semid, shmid1, shmid2);

        if ((shmid2 = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT)) < 0) {
                tst_brkm(TBROK, cleanup, "error in shmget(), %i", errno);
                tst_exit();
        }
        printf("semid: %i shmid1: %i shmid2: %i\n", semid, shmid1, shmid2);
}


void cleanup (void) {
        close(s);
        close(fsd);

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
        printf("SEMA and SHEMA are removed\n");
        
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

        if (server_ready != (int *) -1)
                shmdt(server_ready);
        if (ready_to_kill != (int *)-1)
                shmdt(ready_to_kill);
        exit(0);
}

void setup_server(void)
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
        printf("To wake up\n");
        if (*server_ready) {
                printf("SERer is ready\n");

        } else {
                printf("SERer is NOT ready\n");
        }

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

	/* issue sendto to set up the connections */
	error = sendto(fsd, HELLO_MSG, strlen(HELLO_MSG)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
	if (error < 0) {
                tst_brkm(TBROK, server_cleanup2, "sendto second socket failed"
                       	"test %d", testno);
	} 

	do {
	        new_act.sa_handler = server_alarm_action;

		sigaction(SIGALRM, &new_act, NULL);
		alarm(RUN_TIME);

		error = recvmsg(s, &inmessage, 0);
	
		//printf("server: ");
		//test_print_message(&inmessage, error);

                if (error > 0 && (!(inmessage.msg_flags& MSG_NOTIFICATION))) {
				
                        error = sendto(s, inmessage.msg_iov[0].iov_base,
                                        error, 0, inmessage.msg_name, 
                                        inmessage.msg_namelen);

                } /* if (error > 0) */


                memset(&inmessage, 0, sizeof(inmessage));

                inmessage.msg_namelen = sizeof (struct sockaddr_in6);
                inmessage.msg_name = (void *)(&msg_name);

                in_iov.iov_len = 1024;
                in_iov.iov_base = message;
                inmessage.msg_iov = &in_iov;
                inmessage.msg_iovlen = 1;

                inmessage.msg_control = incmsg;
                inmessage.msg_controllen = sizeof(incmsg); 

        } while (error >= 0 && !(*ready_to_kill)); 

	if (*ready_to_kill) {

		printf("Got killed by parent\n");
	}
	server_cleanup2();
} /* setup_server */


/* 
 * server_alarm_action() - reset the alarm.
 *
 */
void server_alarm_action() {

        new_act.sa_handler = server_alarm_action;
        sigaction (SIGALRM, &new_act, NULL);
        
        alarm(RUN_TIME);
        return; 
}


/* 
 * client_alarm_action() - reset the alarm.
 *
 */
void client_alarm_action() {

        new_act.sa_handler = client_alarm_action;
        sigaction (SIGALRM, &new_act, NULL);
        
        alarm(RUN_TIME);
        return; 
}

