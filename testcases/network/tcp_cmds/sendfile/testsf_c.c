 
/*
 * Client for the send_file test program
 * Syntax: testsf_c <server IP addr> <client_filename> <filename> <file length> <header length> 
 * <trailer length> <flags { N - nonblocking, B - blocking, C - SF_CLOSE, 
 * D - SF_DONT_CACHE, S - SF_SYNC_CACHE, at least one } >
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <sys/socket.h>

extern int errno;

main(argc, argv)
int argc;
char *argv[];

{
  struct sockaddr_in sa;
  int s, fd;
  char *lp, *sp;
  int i;
  int nbyte;
  char *fname;
  char *clnt_fname;
  char rbuf[81];
  int flen, nlen;

  /* open socket to server */
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	printf("socket error = %d\n", errno);
	exit(-1);
  }

  clnt_fname = argv[2]; /* get filename to request */
  fname = argv[3]; /* get filename to request */

  /* prepare to copy file from server to local machine */
  if ((fd = open(clnt_fname, O_CREAT | O_TRUNC | O_WRONLY)) < 0) {
	printf("file open error = %d\n", errno);
	close(s);
	exit(-1);
  }

  lp = argv[4]; /* get file size */
  flen = strtol(lp,(char **)NULL,10);
	

  /* initialize request info: */
  rbuf[0] = '\0';
  sp = &rbuf[0];
  sp = strcat(sp, argv[4]); /* file size */
  sp = strcat(sp, "=");
  sp = strcat(sp, fname); /* requested file */

  /* initialize server info to make the connection */
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(argv[1]);
  sa.sin_port = htons(256);

  if ( connect(s, (struct sockaddr*) &sa, sizeof(sa) ) < 0 ) {
        printf("connect error = %d\n", errno);
	close(s);
	exit(-1);
  }
 
  /* send request info to server */
  if ((nbyte = write(s, rbuf, strlen(rbuf))) <= 0) {
        printf("socket write  error = %d\n", errno);
	close(s);
	exit(-1);
  }

printf("client write %d bytes to server with contents %s\n", nbyte, rbuf);

  nlen = 0; /* init size of info received */
  rbuf[0] = '\0';
  while ((nbyte = read(s, rbuf, 80)) >0) { /* receive info until EOF */
    nlen += nbyte;
    if (write(fd, rbuf, nbyte) != nbyte) {
      printf("Error writing to file %s on client\n",clnt_fname);
      exit(-3);
    }
  }


  if (nlen != flen) { /* compare expected size with current size */
    printf("WRONG!!! nlen = %d, should be %d\n", nlen, flen);
    exit (-2);
  }
  else
    printf("File %s received\n", clnt_fname);

  close(s);
  close(fd);
  exit(0);
}
