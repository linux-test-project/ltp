 
/*
 * Client for the send_file test program
 * Syntax: testsf_c <server IP addr> <client_filename> <filename> <file length> 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

main(argc, argv)
int argc;
char *argv[];

{
  struct sockaddr_in sa;
  int s, fd;
  char *lp, *sp;
  int i;
  int nbyte;
  char *clnt_fname;
  char rbuf[PATH_MAX];
  int flen, nlen;
  int port;

  /* open socket to server */
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	printf("socket error = %d\n", errno);
	exit(1);
  }

  clnt_fname = argv[3]; /* filename to create/
  
  /* prepare to copy file from server to local machine */
  if ((fd = open(clnt_fname, O_CREAT | O_TRUNC | O_WRONLY)) < 0) {
	printf("file open error = %d\n", errno);
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

  printf("sp=%s\n",sp);
  /* initialize server info to make the connection */
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(argv[1]);
  port=atoi(argv[2]);
  sa.sin_port = htons(port);

  if ( connect(s, (struct sockaddr*) &sa, sizeof(sa) ) < 0 ) {
        printf("connect error = %d\n", errno);
	close(s);
	exit(1);
  }
 
  /* send request info to server */
  if ((nbyte = write(s, rbuf, strlen(rbuf))) <= 0) {
        printf("socket write  error = %d\n", errno);
	close(s);
	exit(1);
  }

printf("client write %d bytes to server with contents %s\n", nbyte, rbuf);

  nlen = 0; /* init size of info received */
  rbuf[0] = '\0';
  while ((nbyte = read(s, rbuf, 80)) >0) { /* receive info until EOF */
    nlen += nbyte;
    if (write(fd, rbuf, nbyte) != nbyte) {
      printf("Error writing to file %s on client\n",clnt_fname);
      exit(1);
    }
  }


  if (nlen != flen) { /* compare expected size with current size */
    printf("WRONG!!! nlen = %d, should be %d\n", nlen, flen);
    exit (1);
  }
  else
    printf("File %s received\n", argv[4]);

  close(s);
  close(fd);
  exit(0);
}
