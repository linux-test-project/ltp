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
 *  FILE        : close02-sctp-udp.c
 *  DESCRIPTION : Creates a semaphore and two processes.  The processes 
 *                each go through a loop where they semdown, delay for a
 *                random amount of time, and semup, so they will almost
 *                always be fighting for control of the semaphore.
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

#include "funutil.h"

#define KEY 9145
#define PORT 10001
#define CLIENT_WAIT_TIME 20 
#define SERVER_WAIT_TIME 3 

typedef struct v4_v6 {
        int     has_v4;
        char    v4_addr[64];
        int     has_v6;
        char    v6_addr[64];
        char    if_name[16];
} local_addr_t;

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
int verbose = 0;
int loops = 100;

int semid, shmid1, shmid2;
union semun semunion;
struct shmid_ds shminfo;
int *server_ready, *ready_to_kill;

int s, ssd, fsd;
pid_t pid;

struct sockaddr_in sin1, addr1, addr2, to;

void decode_event(union sctp_notification * sn, int *size, int * got_it); 
void setup(void);
pid_t setup_server(void);
void cleanup(void),  server_cleanup1(void), server_cleanup2(void);
int semup(int semid);
int semdown(int semid);
void client_alarm_action(), server_alarm_action();
int got_shutdown_event(int sd);
void get_ip_addresses(local_addr_t *local_addrs, int * count);

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
                printf( "socket() setup failed: %s",
                        strerror(errno));
		exit(1);
        }

        if (bind(fsd, (struct sockaddr *)&addr1, sizeof(addr1)) < 0) {
                printf( "first client bind failed: %s",
                        strerror(errno));
                exit(1);
        }
}

void second_association() {
	int i;

        ssd = socket(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
        if (ssd < 0) {
                printf( "socket() failed: %s",
                        strerror(errno));
		exit(1);
        }

        if (bind(ssd, (struct sockaddr *)&addr2, sizeof(addr2)) < 0) {
                printf( "second bind failed: %s",
                        strerror(errno));
                exit(1);
        }
}

/*
 * getipckey() - generates and returns a message key used by the "get"
 *               calls to create an IPC resource.
 * taken from kernel/syscalls/ipc/lib/libipc.c
 */
int
getipckey()
{
        const char a = 'a';
        int ascii_a = (int)a;
        char *curdir = NULL;
        size_t size = 0;
        key_t ipc_key;
        struct timeb time_info;

        if (NULL == (curdir = getcwd(curdir, size))) {
                printf( "Can't get current directory "
                         "in getipckey()");
        }

        /*
         * Get a Sys V IPC key
         *
         * ftok() requires a character as a second argument.  This is
         * refered to as a "project identifier" in the man page.  In
         * order to maximize the chance of getting a unique key, the
         * project identifier is a "random character" produced by
         * generating a random number between 0 and 25 and then adding
         * that to the ascii value of 'a'.  The "seed" for the random
         * number is the millisecond value that is set in the timeb
         * structure after calling ftime().
         */
        (void)ftime(&time_info);
        srandom((unsigned int)time_info.millitm);

        if ((ipc_key = ftok(curdir, ascii_a + random()%26)) == -1) {
                printf( "Can't get msgkey from ftok()");
        }

        return(ipc_key);
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

        setup();

	pid = fork();
	
	if (pid == 0) {/* child */
		setup_server();
	} else if (pid == -1) {
		printf( "fork failed. %s", 
			strerror(errno));
	}
	else { /* parent */

		server_ready = (int*) shmat(shmid1, NULL, SHM_RDONLY);
		ready_to_kill = (int*) shmat(shmid2, NULL, 0);

		if ((int *) -1 == server_ready 
		    || (int *) -1 == ready_to_kill) {
			cleanup();
		}

	        if(semdown(semid)) {
			printf("parent: semdown failed\n");       
			exit(1);
		}
		*ready_to_kill = 0;	

		if (!(*server_ready)) {
			printf("SERver is NOT READY\n");
			cleanup();
		} else {
		printf("SERVER IS READY\n");
		}

		first_association();
		second_association();

		/* issue sendto to set up the connections */
		retval = sendto(ssd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
                	printf( "did not get second address"
                        	"test %d", testno);
	                exit(1);
		} 
	
		retval = sendto(fsd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
                	printf( "did not get second address"
                        	"test %d", testno);
	                exit(1);
		} 

		/* 
		 * close the first association to see whether or not the 
		 * second is still alive.
		 */

		close(fsd);
	
		retval = sendto(ssd, hello_msg, strlen (hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
			printf( "close() of multiple assoc returned %d "
				"(expected 0), errno %d (expected 0)", 
				retval, errno); 
			cleanup();
		} 
		else {
			printf( "close() of multiple assoc successful");
		}
	

		/*
		 * 2nd test case: 
		 *  Close server socket to see whether the clients get 
		 *  notified of shutdown event.
		 */
		first_association();
	
		retval = sendto(fsd, hello_msg, strlen (hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
			printf( "close() of multiple assoc returned %d "
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
                	printf( "Client sendto failed "
                        	"test %d", testno);
	                exit(1);
		} 
	
		retval = sendto(fsd, hello_msg, strlen(hello_msg)+1, 0, 
				(struct sockaddr *) &to, sizeof (to));
		if (retval < 0) {
                	printf( "Client sendto failed "
                        	"test %d", testno);
	                exit(1);
		} 
	 	
		printf("ready to kill server\n");
		*ready_to_kill = 1; 

		if (got_shutdown_event(fsd) && got_shutdown_event(ssd)) {
			printf( "got shutdown event as expected");
		} 
		else {
			printf( "close() with multiple assoc "
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
	char v4_dev_name[32];
	int got_first_addr = 0, got_second_addr = 0;
        local_addr_t    local_addrs[10];
	int i, count;

	server_ready = NULL;
	ready_to_kill = NULL;
	
	semid = 0;
	shmid1 = 0;
	shmid2 = 0;

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

        if (!got_second_addr) {
                printf( "did not get second address"
                        "test %d", testno);
                exit(1);
        }

	memset(&addr1, 0, sizeof(addr1));
	addr1.sin_family = AF_INET;
	addr1.sin_addr.s_addr = inet_addr(first_local_addr);
	addr1.sin_port = htons(0);

	memset(&addr2, 0, sizeof(addr2));
	addr2.sin_family = AF_INET;
	addr2.sin_addr.s_addr = inet_addr(second_local_addr);
	addr2.sin_port = htons(0);

	memset(&to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = inet_addr(first_local_addr);
	to.sin_port = htons(PORT);

        /* set up the semaphore */
        if((semid = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT)) < 0) {
                printf( "error in semget()");
                exit(1);
        }
	printf("semid: %i shmid1: %i shmid2: %i\n", semid, shmid1, shmid2);
	
        semunion.val = 0;

        if(semctl(semid, 0, SETVAL, semunion) == -1) {
                printf( "error in semctl()");
		exit(1);
        }
	printf("semid: %i shmid1: %i shmid2: %i\n", semid, shmid1, shmid2);

	if ((shmid1 = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT)) < 0) {
                printf( "error in shmget(), %i", errno);
		exit(1);	
	}
	printf("semid: %i shmid1: %i shmid2: %i\n", semid, shmid1, shmid2);

	if ((shmid2 = shmget(IPC_PRIVATE, sizeof(int), 0777|IPC_CREAT)) < 0) {
                printf( "error in shmget(), %i", errno);
		exit(1);	
	}
	printf("semid: %i shmid1: %i shmid2: %i\n", semid, shmid1, shmid2);
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
	close(ssd);

	if (server_ready != (int *) -1)
		shmdt(server_ready);
	if (ready_to_kill != (int *)-1)
		shmdt(ready_to_kill);
	exit(0);
}

pid_t setup_server(void)
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

	server_ready = (int *) shmat(shmid1, 0, SHM_W|SHM_R);	
	ready_to_kill = (int *) shmat(shmid2, 0, SHM_RDONLY);

	if ((int *)-1 == server_ready  || (int *) -1 ==  ready_to_kill ) {
                printf( 
			"server shmat failed: %s",
                        strerror(errno));
        }

	*server_ready = 1; 

        /* initialize sockaddr's */
        sin1.sin_family = AF_INET;
        sin1.sin_port = htons(PORT);
  //      sin1.sin_addr.s_addr = inet_addr("1.2.3.4");
        sin1.sin_addr.s_addr = INADDR_ANY;

	s = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	
        if (s < 0) {
		*server_ready = 0;
                printf( "socket setup failed: %s",
                        strerror(errno));
        } 
	else if (bind(s, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
		*server_ready = 0;
                printf( "server bind failed: %s",
                        strerror(errno));		
        }
        else if (listen(s, 10) < 0) {
		*server_ready = 0;
                printf( "server listen failed: %s",
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
                printf( "server setup failed: %s",
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
		alarm(SERVER_WAIT_TIME);
	        error = recvmsg(s, &inmessage, 0);

		printf("server: ");
		if (errno != 4)
		test_print_message(&inmessage, error);

                if (errno != 4 && error > 0 && (!(inmessage.msg_flags& MSG_NOTIFICATION))) {
			
			printf("Server sendto: %s\n", 
				inmessage.msg_iov[0].iov_base);	
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
		printf("ready to die: %i\n", *ready_to_kill);
		
		if (errno == 4) {
			error = 0;
		}
        } while (error >= 0 && !(*ready_to_kill)); 
	
	if (*ready_to_kill) {
		printf("got killed by parent\n");
	} 
	server_cleanup2();

} /* setup_server */


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

		printf("Waiting for shutdown event.\n");
	        error = recvmsg(sd, &inmessage, 0);
	        if (error < 0) { 
			break;	
		}
		printf("Client: ");
		test_print_message(&inmessage, error);

	        for (scmsg = CMSG_FIRSTHDR(&inmessage);
		     scmsg != NULL;
		     scmsg = CMSG_NXTHDR(&inmessage, scmsg)) {
			type = scmsg->cmsg_type;
			data = (sctp_cmsg_data_t *)CMSG_DATA(scmsg);
			test_print_cmsg(scmsg->cmsg_type, data);
		}


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

        //new_act.sa_handler = client_alarm_action;
        //sigaction (SIGALRM, &new_act, NULL);
        
        //alarm(CLIENT_WAIT_TIME);
	*ready_to_kill = 1;
        return; 
}

void decode_event(union sctp_notification * sn, int *size, int * got_it) { 

	printf("type: %x\n", sn->h.sn_type);
        if (SCTP_ASSOC_CHANGE == sn->h.sn_type) { 
		if (SHUTDOWN_COMPLETE== sn->sn_assoc_change.sac_state) {
			printf("Got SHUTDOWN_COMPLETE");
	                *size = sizeof (struct sctp_assoc_change);      
       	        	*got_it = 1; 
		}
		printf("Got ASSOC_CHANGE\n");
        } 
        else if (SCTP_PEER_ADDR_CHANGE== sn->h.sn_type) {
                *size = sizeof (struct sctp_paddr_change);
        }  
        else if (SCTP_REMOTE_ERROR== sn->h.sn_type) {
                *size = sizeof (struct sctp_remote_error);
		printf("REMOTE error\n");
        } 
        else if (SCTP_SEND_FAILED== sn->h.sn_type) {
                *size = sizeof (struct sctp_send_failed);
		printf("SEND failed\n");
        }  
        else if (SCTP_SHUTDOWN_EVENT== sn->h.sn_type) {
                *size = sizeof (struct sctp_shutdown_event);
               	*got_it = 1; 
		printf("Got SHUTDOWN_EVENT");
        } 
        /*else if (SCTP_ADAPTION_INDICATION== sn->h.sn_type) {
                *size = sizeof (struct sctp_adaption_indication;
        }
        else if (SCTP_PARTIAL_DELIVERY_EVENT== sn->h.sn_type) {
                *size = sizeof (struct sctp_partial_delivery_event);
        }*/
} /* decode_event */

/*
 * Read the interfaces from file /proc/net/dev and /proc/net/if_inet6.
 * Retrieve the IPv4 or IPv6 addresses for the interfaces. 
 *
 */
void get_ip_addresses(local_addr_t *local_addrs, int * count)
{

        int fd; 
        FILE *inFile;
        int *x; 
        struct ifreq ifr;

        struct in_addr z; 
        char buf[16];
        char p[16]; 
        char *pos;

        char address[64];

        int i = 0;

        if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
                printf( "Can't talk to kernel! (%d)\n", errno);
                exit(1);
        } 

        inFile = fopen("/proc/net/dev", "r");
        if (!inFile) {
                printf( "Unable to open file /proc/net/dev.\n");
                exit(1);
        }

        while ( fscanf(inFile, "%s\n", buf) == 1) {

                *p = 0; /* remove old address  */
                if (!(pos=(char*)strchr(buf, ':'))) {
                        continue;
                } else {
                        *pos = '\0';    
                } 
                strcpy(local_addrs[i].if_name, buf);     
                //printf("if_name: %s\n", buf);
                strcpy(ifr.ifr_name, buf); 
        
        
                if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) { 
                        printf( "Can't get status for %s. (%d)\n", 
                                buf, errno);
                        close(fd); 
                        exit(1);

                } 

                if ((ifr.ifr_flags & IFF_UP) == 0) { 
                        continue; 
                } 

                if (ioctl(fd, SIOCGIFADDR, &ifr) != 0) { 
                        printf( 
                                "Can't get IP address for %s. (%d)\n", 
                                buf, errno);
                        close(fd); 
                        exit(1);
                } 

                x = (int *)&ifr.ifr_addr; // seems to be off by 2 bytes 
                z.s_addr = x[1]; 
                strcpy(p, (char*)inet_ntoa(z)); 

                if (p != NULL) {
                        local_addrs[i].has_v4 = 1;
                        strcpy(local_addrs[i].v4_addr, p);
                } else {
                        local_addrs[i].has_v4 = 0;
                        local_addrs[i].v4_addr[0] = '\0';
                }
                //printf("addr: %s\n", local_addrs[i].v4_addr);
                i++;
        }  /* while */

        *count = i;
        fclose(inFile);

        inFile = fopen("/proc/net/if_inet6", "r");
        if (!inFile) {
                printf( "Unable to open file /proc/net/if_inet6.\n");
                exit(1);
        }

        while (fscanf(inFile, "%s\n", address) == 1) {

                int j = 0;
                char *addr_ptr;
                char colon_addr[64];
                int has_double_colon = 0;

                addr_ptr = &address[0];

                colon_addr[0] = '\0';
        
                while (*addr_ptr){
                        char tmp_str[6];
        
                        strncpy(&tmp_str[0], addr_ptr, 4);
                        tmp_str[4] = '\0';

                        strcat(tmp_str, ":");
                        strcat(colon_addr, tmp_str);
        
                        addr_ptr += 4;  
                }
                colon_addr[strlen(colon_addr)-1] = '\0';

                *p = 0; /* remove old address  */

                fscanf(inFile, "%s\n", buf);
                fscanf(inFile, "%s\n", buf);
                fscanf(inFile, "%s\n", buf);
                fscanf(inFile, "%s\n", buf);
                fscanf(inFile, "%s\n", buf);

                for (i = 0; i < *count; i++) {
                        if (!strcmp(buf, local_addrs[i].if_name)){
                                local_addrs[i].has_v6 = 1;
                                strcpy(local_addrs[i].v6_addr, colon_addr);
                                break;
                        }
                        
                }       

                if (i >= *count) {

                        local_addrs[*count].has_v6 = 1;
                        local_addrs[*count].has_v4 = 0;
                        local_addrs[*count].v4_addr[0] = '\0';
                        strcpy(local_addrs[*count].v6_addr, colon_addr);
                        (*count)++;
                }
        }

        fclose(inFile);
        close(fd); 
} /* get_ip_addresses */


