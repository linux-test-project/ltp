 
/*
 * Client for the send_file test program
 * Syntax: testsf_c <server IP addr> <client_filename> <server_filename> <file length> 
 */

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
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
  struct sockaddr_in6 sa;
  int s, fd;
  char *lp, *sp;
  int i;
  int nbyte;
  char *serv_fname;
  char *clnt_fname;
  char rbuf[81];
  int flen, nlen;
  int gai;
  struct  addrinfo *hp;
  struct  addrinfo hints;
  int port;

  /* open socket to server */
  if ((s = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
	printf("socket error = %d\n", errno);
	exit(1);
  }

  clnt_fname = argv[3]; /* filename to create */
  serv_fname = argv[4]; /* filename to request */

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
  sp = strcat(sp, serv_fname); /* requested file */

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_INET6;
  if ((gai=getaddrinfo(argv[1], NULL, &hints, &hp))!=0)
  		 fprintf(stderr, "Unknown subject address %s: %s\n",argv[1], gai_strerror(gai));
  if (!hp->ai_addr || hp->ai_addr->sa_family != AF_INET6)
  		 fprintf(stderr, "getaddrinfo failed");


  /* initialize server info to make the connection */
  memcpy(&sa, hp->ai_addr, hp->ai_addrlen);
  port=atoi(argv[2]);
  sa.sin6_port = htons(port);

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

  printf("Asking for %s\n",serv_fname);
  if (nlen != flen) { /* compare expected size with current size */
    printf("WRONG!!! nlen = %d, should be %d\n", nlen, flen);
    exit (1);
  }
  else
    printf("File %s received\n", serv_fname);

  close(s);
  close(fd);
  exit(0);
}
