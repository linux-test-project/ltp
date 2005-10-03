 
/*
 * Client for the send_file test program
 * Syntax: testsf_c <server IP addr> <client_filename> <filename> <file length> 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"


#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

char *TCID="sendfile";
int TST_TOTAL=1;
extern int Tst_count;

int main(argc, argv)
int argc;
char *argv[];

{
  struct sockaddr_in sa;
  int s, fd;
  char *lp, *sp;
  int nbyte;
  char *clnt_fname;
  char rbuf[PATH_MAX];
  int flen, nlen;
  int port;

  /* open socket to server */
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	tst_resm(TBROK, "socket error = %d\n", errno);
	exit(1);
  }

  clnt_fname = argv[3]; /* filename to create */
  
  /* prepare to copy file from server to local machine */
  if ((fd = open(clnt_fname, O_CREAT | O_TRUNC | O_WRONLY)) < 0) {
	tst_resm(TBROK, "file open error = %d\n", errno);
	close(s);
	exit(1);
  }

  lp = argv[5]; /* get file size */
  flen = strtol(lp,(char **)NULL,10);
	

  /* initialize request info: */
  rbuf[0] = '\0';
  sp = &rbuf[0];
  sp = strcat(sp, argv[5]); /* file size */
  sp = strcat(sp, "=");
  sp = strcat(sp, argv[4]); /* requested file */

  tst_resm(TINFO, "sp=%s\n",sp);
  /* initialize server info to make the connection */
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(argv[1]);
  port=atoi(argv[2]);
  sa.sin_port = htons(port);

  if ( connect(s, (struct sockaddr*) &sa, sizeof(sa) ) < 0 ) {
        tst_resm(TBROK, "connect error = %d\n", errno);
	close(s);
	exit(1);
  }
 
  /* send request info to server */
  if ((nbyte = write(s, rbuf, strlen(rbuf))) <= 0) {
        tst_resm(TBROK, "socket write  error = %d\n", errno);
	close(s);
	exit(1);
  }

  tst_resm(TINFO, "client write %d bytes to server with contents %s\n",
	   nbyte, rbuf);

  nlen = 0; /* init size of info received */
  rbuf[0] = '\0';
  while ((nbyte = read(s, rbuf, PATH_MAX)) >0) { /* receive info until EOF */
    nlen += nbyte;
    if (write(fd, rbuf, nbyte) != nbyte) {
      tst_resm(TBROK, "Error writing to file %s on client\n",clnt_fname);
      exit(1);
    }
  }


  if (nlen != flen) { /* compare expected size with current size */
    tst_resm(TBROK, "WRONG!!! nlen = %d, should be %d\n", nlen, flen);
    exit (1);
  }
  else
    tst_resm(TINFO, "File %s received\n", argv[4]);

  close(s);
  close(fd);
  exit(0);
}
