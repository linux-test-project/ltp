/*
 * Server for the sendfile test program
 * Syntax: testsf_s <own IP addr>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <errno.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"


#define LISTEN_BACKLOG	10

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

char *TCID="sendfile";
int TST_TOTAL=1;
extern int Tst_count;


void 
test_sig(sig)
int sig;
{
  
  int status;

  wait(&status);


}


int main(argc, argv)
int argc;
char *argv[];

{
  struct sockaddr_in sa, *ap;
  struct sockaddr from;
  int s, fd, as, rc;
  char *lp;
  char *number;
  int clen, pid;
  int nbytes, flen,count;
  char rbuf[PATH_MAX];
  int chunks=0;
  off_t *offset; 
  char nbuf[PATH_MAX];
  int port;
 
  /* open socket */
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	tst_resm(TBROK, "socket error = %d\n", errno);
	exit(-1);
  }

  signal(SIGCHLD, SIG_IGN); /* ignore signals from children */

  /* server IP and port */
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(argv[1]);
  port=atoi(argv[2]);
  sa.sin_port = htons(port);

  /* bind IP and port to socket */
  if ( bind(s, (struct sockaddr*) &sa, sizeof(sa) ) < 0 ) {
        tst_resm(TBROK, "bind error = %d\n", errno);
	close(s);
	exit(-1); 
  }
 
  /* start to listen socket */
  if ( listen(s, LISTEN_BACKLOG ) < 0 ) {
        tst_resm(TBROK, "listen error = %d\n", errno);
	close(s);
	exit(-1);
  }

  /* process connections */
  while(1) {
	/* accept a connection from a client */
  	clen = sizeof(from);
  	if ((as = accept(s, &from, &clen )) < 0 ) {
	  tst_resm(TBROK, "accept error = %d\n", errno);
	  if (errno == EINTR)
		continue;
	  close(s);
	  exit(-1);
  	}

	ap = (struct sockaddr_in *)&from;

	/* create a process to manage the connection */
	if ((pid = fork()) < 0) {
	  tst_resm(TBROK, "fork error = %d\n", errno);
	  close(as);
	  exit(-1);
  	}
	if (pid > 0) {	/* parent, go back to accept */
	  close(as);
	  continue;
	}



	/* child process to manage a connection */
  
	close(s); /* close service socket */

	/* get client request information */
	if ((nbytes = read(as, rbuf, PATH_MAX)) <= 0) {
	  tst_resm(TBROK, "socket read error = %d\n", errno);
	  close(as);
	  exit(-1);
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

	/* the file name */
	lp++;

        tst_resm(TINFO, "The file to send is %s\n", lp);
  	/* open requested file to send */
	if ((fd = open(lp, O_RDONLY)) < 0) {
	  tst_resm(TBROK, "file open error = %d\n", errno);
	  close(as);
	  exit(-1);
  	}
	offset=NULL;
	errno=0;
        do { /* send file parts until EOF */
           if ((rc = sendfile(as, fd, offset, flen)) != flen) {
                if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
                        tst_resm(TBROK, "sendfile error = %d, rc = %d\n", errno, rc);
                        close(as);
                        close(fd);
                        exit(-1);
                }
          }
	  chunks++;
        } while (rc != 0);
	tst_resm(TINFO, "File %s sent in %d parts\n", lp, chunks);


	close(as); /* close connection */
	close(fd); /* close requested file */
	return(0);

  }
  close(s); /* close parent socket (never reached because of the while(1)) */
}
