/*
 * Server for the sendfile test program
 * Syntax: testsf_s <own IP addr>
 */

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <errno.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define LISTEN_BACKLOG	10


void *
test_sig(sig)
int sig;
{
  
  int status;

  wait(&status);


}


main(argc, argv)
int argc;
char *argv[];

{
  struct sockaddr_in6 sa, *ap;
  struct sockaddr_in6 from;
  struct  addrinfo *hp;
  struct  addrinfo hints;
  int s, fd, as, rc;
  char *lp;
  char *number;
  int i, clen, pid;
  int flags, nonblocking;
  int nbytes, flen,gai,count;
  char rbuf[81];
  int chunks=0;
  off_t *offset; 
  char nbuf[81];
  int port;
 
  /* open socket */
  if ((s = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
	printf("socket error = %d\n", errno);
	exit(1);
  }

  signal(SIGCHLD, SIG_IGN); /* ignore signals from children */

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_INET6;
  if ((gai=getaddrinfo(argv[1], NULL, &hints, &hp))!=0)
        fprintf(stderr, "Unknown subject address %s: %s\n",argv[1], gai_strerror(gai));
  if (!hp->ai_addr || hp->ai_addr->sa_family != AF_INET6)
        fprintf(stderr, "getaddrinfo failed");


  /* server IP and port */
  memcpy(&sa, hp->ai_addr, hp->ai_addrlen);
  port=atoi(argv[2]);
  sa.sin6_port = htons(port);

  /* bind IP and port to socket */
  if ( bind(s, (struct sockaddr_in6*) &sa, sizeof(sa) ) < 0 ) {
        printf("bind error = %d\n", errno);
	close(s);
	exit(1);
  }
 
  /* start to listen socket */
  if ( listen(s, LISTEN_BACKLOG ) < 0 ) {
        printf("listen error = %d\n", errno);
	close(s);
	exit(1);
  }

  /* process connections */
  while(1) {
	/* accept a connection from a client */
  	clen = sizeof(from);
  	if ((as = accept(s, &from, &clen )) < 0 ) {
	  printf("accept error = %d\n", errno);
	  if (errno == EINTR)
		continue;
	  close(s);
	  exit(1);
  	}

	ap = (struct sockaddr_in6 *)&from;

	/* create a process to manage the connection */
	if ((pid = fork()) < 0) {
	  printf("fork error = %d\n", errno);
	  close(as);
	  exit(1);
  	}
	if (pid > 0) {	/* parent, go back to accept */
	  close(as);
	  continue;
	}



	/* child process to manage a connection */
  
	close(s); /* close service socket */

	/* get client request information */
	if ((nbytes = read(as, rbuf, 80)) <= 0) {
	  printf("socket read error = %d\n", errno);
	  close(as);
	  exit(1);
  	}
	rbuf[nbytes] = '\0'; /* null terminate the info */
	lp = &rbuf[0];

	/* start with file length, '=' will start the filename */
	flen = 0;
	count = 0;
	number = &nbuf[0];
	while (*lp != '=') { /* convert ascii to integer */
	  nbuf[count] = *lp;
	  count++;
	  lp++;
	}
	 nbuf[count] = '\0';
	 flen = strtol(number, (char **)NULL, 10); 
	 printf("flen is %d.\n",flen);

	/* the file name */
	lp++;

        printf("The file to send is %s\n", lp);
  	/* open requested file to send */
	if ((fd = open(lp, O_RDONLY)) < 0) {
	  printf("file open error = %d\n", errno);
	  close(as);
	  exit(1);
  	}
	offset=NULL;
	errno=0;
        do { /* send file parts until EOF */
           if ((rc = sendfile(as, fd, offset, flen)) != flen) {
                if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
                        printf("sendfile error = %d, rc = %d\n", errno, rc);
                        close(as);
                        close(fd);
                        exit(-1);
                }
          }
	  chunks++;
        } while (rc != 0);
	printf("File %s sent in %d parts\n", lp, chunks);


	close(as); /* close connection */
	close(fd); /* close requested file */
	exit(0);

  }


  close(s); /* close parent socket (never reached because of the while(1)) */




}

