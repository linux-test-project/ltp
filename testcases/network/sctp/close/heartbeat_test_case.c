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
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include "funutil.h"

#define PORT 10001
#define RUN_TIME 5 

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

int semid;
int s, ssd, fsd;
pid_t pid;

struct sockaddr_in sin1, addr1, addr2, to;

void decode_event(union sctp_notification * sn, int *size, int * got_it); 
void setup(void);
pid_t setup_server(void);
void cleanup(void);
int semup(int semid);
int semdown(int semid);
void alarm_action();
void terminate_server(void);
int got_shutdown_event(int sd);
void get_ip_addresses(local_addr_t *local_addrs, int * count);

int semup(int semid) {
	struct sembuf semops;
	semops.sem_num = 0;
	semops.sem_op = 1;
	semops.sem_flg = SEM_UNDO;
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
	semops.sem_flg = SEM_UNDO;
	if(semop(semid, &semops, 1) == -1) {
		perror("semdown");
		return 1;
	}
	return 0;
}

void first_association(int semid) {
	int i;

	if(semdown(semid)) {
		printf("semdown failed\n");
	}

        fsd = socket(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
        if (fsd < 0) {
                printf("socket setup failed: %s",
                        strerror(errno));
        }

        if (bind(fsd, (struct sockaddr *)&addr1, sizeof(addr1)) < 0) {
                printf("second bind failed: %s",
                        strerror(errno));
                return;
        }
}

void second_association(int semid) {
	int i;

	if(semup(semid)) {
		printf("sem is up\n");
	}

        ssd = socket(PF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
        if (ssd < 0) {
                printf("socket setup failed: %s",
                        strerror(errno));
		return;
        }

        if (bind(ssd, (struct sockaddr *)&addr2, sizeof(addr2)) < 0) {
                printf("child bind failed: %s",
                        strerror(errno));
                return;
        }
}


int main(int argc, char *argv[]) {
	int semid, opt;
	union semun semunion;
	char *msg;
	pid_t pid;
	int chstat;
	int lc;
	int retval;
	struct sctp_event_subscribe event;
	char hello_msg[] = "HELLO! ARE YOU STILL THERE";

        /* Parse standard options given to run the test. */
       // msg = parse_opts(argc, argv, (option_t *)NULL, NULL);
       // if (msg != (char *)NULL) {
        //        printf("OPTION PARSING ERROR - %s", msg);
        //}

        setup();
        /* set up the semaphore */
        if((semid = semget((key_t)9142, 1, 0666 | IPC_CREAT)) < 0) {
                printf("error in semget()\n");
                exit(-1);
        }
        semunion.val = 1;

        if(semctl(semid, 0, SETVAL, semunion) == -1) {
                printf("error in semctl\n");
        }


	/* set up the semaphore */
	if((semid = semget((key_t)9142, 1, 0666 | IPC_CREAT)) < 0) {
		printf("error in semget()\n");
		exit(-1);
	}
	semunion.val = 1;
	if(semctl(semid, 0, SETVAL, semunion) == -1) {
		printf("error in semctl\n");
	}

	switch (pid = fork()) {
	case 0: /* child */
		setup_server();
		break;

	case -1: 
		printf("server fork failed. %s", strerror(errno));
		/* fall through */	
	default: /* parent */
		break;
	}

	first_association(semid);
	second_association(semid);

	/* issue sendto to set up the connections */
	retval = sendto(ssd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
	if (retval < 0) {
		printf( "close() of multiple assoc returned %d "
			"(expected 0), errno %d (expected 0)", retval, errno); 
		cleanup();
	} 
	
	retval = sendto(fsd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
	if (retval < 0) {
		printf("close() of multiple assoc returned %d "
			"(expected 0), errno %d (expected 0)", retval, errno); 
		cleanup();
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
			"(expected 0), errno %d (expected 0)", retval, errno); 
		cleanup();
	} 
	else {
		printf( "close() of multiple assoc successful");
	}
	
	second_association(semid);

	/*
	 * Close server socket to see whether the clients get notified of 
	 * shutdown event.
	 */ 
	terminate_server();

	/* turn on shutdown event on both client sockets */
	memset(&event, 0, sizeof(event));
	event.sctp_shutdown_event = 1;

/*	if (setsockopt(fsd, IPPROTO_SCTP, SCTP_SET_EVENT, 
		&event, sizeof(event)) < 0) {
		perror("setsockopt SCTP_SET_EVENT");
	}	

	if (setsockopt(ssd, IPPROTO_SCTP, SCTP_SET_EVENT, 
		&event, sizeof(event)) < 0) {
		perror("setsockopt SCTP_SET_EVENT");
	}

	if (sendto(ssd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to)) > 0 
	    || sendto(fsd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to)) > 0) {
		printf( "server close() with multiple assoc "
			"returned %d (expected 0), errno %d (expected 0)", 
			retval, errno); 
		cleanup();
	
	}
*/

	/* issue sendto to set up the connections */
	retval = sendto(ssd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
	if (retval < 0) {
		printf( "close() of multiple assoc returned %d "
			"(expected 0), errno %d (expected 0)", retval, errno); 
		cleanup();
	} 
	
	retval = sendto(fsd, hello_msg, strlen(hello_msg)+1, 0, 
			(struct sockaddr *) &to, sizeof (to));
	if (retval < 0) {
		printf( "close() of multiple assoc returned %d "
			"(expected 0), errno %d (expected 0)", retval, errno); 
		cleanup();
	} 
	 	
	if (got_shutdown_event(fsd) && got_shutdown_event(ssd)) {
                printf( "close() of multiple assoc successful");
	} 
	else {
		printf( "server close() with multiple assoc "
			"returned %d (expected 0), errno %d (expected 0)", 
			retval, errno); 
		cleanup();
	}
		
}

void
setup(void)
{
	char first_local_addr[128], second_local_addr[128];
	char v4_dev_name[32];
	int got_first_addr = 0, got_second_addr = 0;
        local_addr_t    local_addrs[10];
	int i, count;


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
                printf("did not get second address"
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
}

void cleanup (void) {
	close(s);
	close(fsd);
	close(ssd);
	printf ("SERVER KILLED");
	(void) kill (pid, SIGKILL);

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
	
        /* initialize sockaddr's */
        sin1.sin_family = AF_INET;
        sin1.sin_port = htons(PORT);
        sin1.sin_addr.s_addr = INADDR_ANY;

        s = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
        if (s < 0) {
                printf( "socket setup failed: %s",
                        strerror(errno));
        }

        if (bind(s, (struct sockaddr *)&sin1, sizeof(sin1)) < 0) {
                printf( "server bind failed: %s",
                        strerror(errno));
                return;
        }
        if (listen(s, 10) < 0) {
                printf( "server listen failed: %s",
                        strerror(errno));
                return;
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


        new_act.sa_handler = alarm_action;

        sigaction(SIGALRM, &new_act, NULL);
        alarm(RUN_TIME);

        printf("in server, waiting for message\n");
        //timestamp = (char *) ctime(&tm);
        error = recvmsg(s, &inmessage, 0);
        //printf("server: ");
        //test_print_message(&inmessage, error);

        do {

                if (error > 0 && (!(inmessage.msg_flags & MSG_NOTIFICATION))) {
			if (inmessage.msg_name == NULL 
				|| inmessage.msg_namelen == 0) {
				printf("MSG NAME IS NULL\n");
				exit(0);	
			}
				
                        error = sendto(s, inmessage.msg_iov[0].iov_base,
                                        error, 0, inmessage.msg_name, 
                                        inmessage.msg_namelen);
                                printf("Server, sendto: %s\n", 
                                        inmessage.msg_iov[0].iov_base);


                } /* if (error > 0) */


                //timestamp = (char *) ctime(&tm);
                error = recvmsg(s, &inmessage, 0);
               // printf("server: ");
               // test_print_message(&inmessage, error);
                memset(&inmessage, 0, sizeof(inmessage));

                inmessage.msg_namelen = sizeof (struct sockaddr_in6);
                inmessage.msg_name = (void *)(&msg_name);

                in_iov.iov_len = 1024;
                in_iov.iov_base = message;
                inmessage.msg_iov = &in_iov;
                inmessage.msg_iovlen = 1;

                inmessage.msg_control = incmsg;
                inmessage.msg_controllen = sizeof(incmsg); 

        } while (error >= 0); 


} /* setup_server */

void terminate_server(void) {
	close(s);

}

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
        new_act.sa_handler = alarm_action;

        sigaction(SIGALRM, &new_act, NULL);
        alarm(RUN_TIME);

	got_it = 0;
	while (!got_it) {
		int error;

		printf("Waiting for shutdown event.\n");
	        error = recvmsg(sd, &inmessage, 0);
		printf("Client: ");
		test_print_message(&inmessage, error);
	        if (error < 0) { 
			break;	
		}
	        for (scmsg = CMSG_FIRSTHDR(&inmessage);
		     scmsg != NULL;
		     scmsg = CMSG_NXTHDR(&inmessage, scmsg)) {
			type = scmsg->cmsg_type;
			data = (sctp_cmsg_data_t *)CMSG_DATA(scmsg);
		//	test_print_cmsg(scmsg->cmsg_type, data);
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
 * alarm_action() - reset the alarm.
 *
 */
void alarm_action() {

        new_act.sa_handler = alarm_action;
        sigaction (SIGALRM, &new_act, NULL);
        
        alarm(RUN_TIME);
        return; 
}

void decode_event(union sctp_notification * sn, int *size, int * got_it) { 

        if (SCTP_ASSOC_CHANGE == sn->h.sn_type) {
                *size = sizeof (struct sctp_assoc_change);      
               	*got_it = 1; 
        } 
        else if (SCTP_PEER_ADDR_CHANGE== sn->h.sn_type) {
                *size = sizeof (struct sctp_paddr_change);
        }  
        else if (SCTP_REMOTE_ERROR== sn->h.sn_type) {
                *size = sizeof (struct sctp_remote_error);
        } 
        /*else if (SCTP_SEND_FAILED== sn->h.sn_type) {
                *size = sizeof (struct sctp_send_failed);
        } */ 
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
                printf( 0, "Can't talk to kernel! (%d)\n", errno);
                exit(1);
        } 

        inFile = fopen("/proc/net/dev", "r");
        if (!inFile) {
                printf( 0, "Unable to open file /proc/net/dev.\n");
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
                        printf( 0, "Can't get status for %s. (%d)\n", 
                                buf, errno);
                        close(fd); 
                        exit(1);

                } 

                if ((ifr.ifr_flags & IFF_UP) == 0) { 
                        continue; 
                } 

                if (ioctl(fd, SIOCGIFADDR, &ifr) != 0) { 
                        printf( 0, 
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
                printf("Unable to open file /proc/net/if_inet6.\n");
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


