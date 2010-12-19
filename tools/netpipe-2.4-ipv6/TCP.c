/*****************************************************************************/
/* "NetPIPE" -- Network Protocol Independent Performance Evaluator.          */
/* Copyright 1997, 1998 Iowa State University Research Foundation, Inc.      */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation.  You should have received a copy of the     */
/* GNU General Public License along with this program; if not, write to the  */
/* Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.   */
/*                                                                           */
/*     * TCP.c              ---- TCP calls source                            */
/*     * TCP.h              ---- Include file for TCP calls and data structs */
/* 2002/03/18 --- Modified for IPv6 - Robbie Williamson (robbiew@us.ibm.com) */
/*****************************************************************************/
#include    "netpipe.h"


int Setup(ArgStruct *p)
{

 int tr, one = 1;                 /* tr==1 if process is a transmitter */
 int sr = 0;
 int sockfd;
 struct sockaddr *lsin1;
 char *host;
 char *server_host;
 struct addrinfo *addr;
 struct addrinfo *server_addr;
 struct addrinfo hints;
 struct protoent *proto;

 memset(&hints, 0, sizeof(hints));
 hints.ai_family = AF_INET6;


 host = p->host;                           /* copy ptr to hostname */
 server_host = p->server_host;             /* copy ptr to server */

 tr = p->tr;                               /* copy tr indicator */
 sr = p->sr;


 memset(&p->prot.sin1, 0, sizeof(p->prot.sin1));
 memset(&p->prot.sin2, 0, sizeof(p->prot.sin2));

 if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
   printf("NetPIPE: can't open stream socket! errno=%d\n", errno);
   exit(-4);
 }

 if (!(proto = getprotobyname("tcp"))) {
   printf("NetPIPE: protocol 'tcp' unknown!\n");
   exit(555);
 }

 /* Attempt to set TCP_NODELAY */
 if (setsockopt(sockfd, proto->p_proto, TCP_NODELAY, &one, sizeof(one)) < 0)
 {
   printf("NetPIPE: setsockopt: TCP_NODELAY failed! errno=%d\n", errno);
   exit(556);
 }

 /* If requested, set the send and receive buffer sizes */
 if (p->prot.sndbufsz > 0)
 {
      printf("Send and Receive Buffers set to %d bytes\n", p->prot.sndbufsz);
     if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &(p->prot.sndbufsz),
                                       sizeof(p->prot.sndbufsz)) < 0)
     {
          printf("NetPIPE: setsockopt: SO_SNDBUF failed! errno=%d\n", errno);
          exit(556);
     }
     if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &(p->prot.rcvbufsz),
                                       sizeof(p->prot.rcvbufsz)) < 0)
     {
          printf("NetPIPE: setsockopt: SO_RCVBUF failed! errno=%d\n", errno);
          exit(556);
     }
 }

 if (tr) {                                  /* if client i.e., Sender */

   if (host) {
     getaddrinfo(host, NULL, &hints, &addr);
     memcpy(&p->prot.sin1, addr->ai_addr, addr->ai_addrlen);
   } else {
     if ((getaddrinfo(host,NULL,&hints,&addr)) != 0) {
       printf("NetPIPE: invalid hostname '%s'\n", host);
       exit(-5);
   }
 }
   p->prot.sin1.sin6_port = htons(p->port);

} else {                                 /* we are the receiver (server) */

   memset(&p->prot.sin1, 0, sizeof(p->prot.sin1));
   if (sr==0) {
     p->prot.sin1.sin6_addr = in6addr_any;
     p->prot.sin1.sin6_port = htons(p->port);
     lsin1=(struct sockaddr*)&p->prot.sin1;
     if (bind(sockfd, (struct sockaddr*)lsin1, sizeof(p->prot.sin1)) < 0) {
       printf("NetPIPE: server: bind on local address failed! errno=%d", errno);
       exit(-6);
     }
   }
   else{
       getaddrinfo(server_host, NULL, NULL, &server_addr);
       memcpy(&p->prot.sin1, server_addr->ai_addr, server_addr->ai_addrlen);
       if ((getaddrinfo(server_host, NULL, NULL, &server_addr)) != 0) {
  	  printf("NetPIPE: invalid hostname '%s'\n", host);
	  exit(-5);
       }
       memcpy(&p->prot.sin1, server_addr->ai_addr, server_addr->ai_addrlen);
       p->prot.sin1.sin6_port = htons(p->port);
       lsin1=(struct sockaddr*)&p->prot.sin1;
       if (bind(sockfd, (struct sockaddr*)lsin1, sizeof(p->prot.sin1)) < 0) {
         printf("NetPIPE: server: bind on %s failed! errno=%d", server_host, errno);
         exit(-6);
       }
   }
 }

 if (tr)
   p->commfd = sockfd;
 else
   p->servicefd = sockfd;

 return(0);

}

static int
readFully(int fd, void *obuf, int len)
{
  int bytesLeft = len;
  char *buf = (char *) obuf;
  int bytesRead = 0;

  while (bytesLeft > 0 &&
	 (bytesRead = read(fd, (void *) buf, bytesLeft)) > 0)
    {
      bytesLeft -= bytesRead;
      buf += bytesRead;
    }
  if (bytesRead <= 0)
    return bytesRead;
  return len;
}

void Sync(ArgStruct *p)
{
    char s[] = "SyncMe";
    char response[7];

    if (write(p->commfd, s, strlen(s)) < 0 ||
	readFully(p->commfd, response, strlen(s)) < 0)
      {
	perror("NetPIPE: error writing or reading synchronization string");
	exit(3);
      }
    if (strncmp(s, response, strlen(s)))
      {
	fprintf(stderr, "NetPIPE: Synchronization string incorrect!\n");
	exit(3);
      }
}

void PrepareToReceive(ArgStruct *p)
{
	/*
	  The Berkeley sockets interface doesn't have a method to pre-post
	  a buffer for reception of data.
	*/
}

void SendData(ArgStruct *p)
{
    int bytesWritten, bytesLeft;
    char *q;

    bytesLeft = p->bufflen;
    bytesWritten = 0;
    q = p->buff;
    while (bytesLeft > 0 &&
	   (bytesWritten = write(p->commfd, q, bytesLeft)) > 0)
      {
	bytesLeft -= bytesWritten;
	q += bytesWritten;
      }
    if (bytesWritten == -1)
      {
	printf("NetPIPE: write: error encountered, errno=%d\n", errno);
	exit(401);
      }
}

void RecvData(ArgStruct *p)
{
    int bytesLeft;
    int bytesRead;
    char *q;

    bytesLeft = p->bufflen;
    bytesRead = 0;
    q = p->buff1;
    while (bytesLeft > 0 &&
	   (bytesRead = read(p->commfd, q, bytesLeft)) > 0)
      {
	bytesLeft -= bytesRead;
	q += bytesRead;
      }
    if (bytesLeft > 0 && bytesRead == 0)
      {
	printf("NetPIPE: \"end of file\" encountered on reading from socket\n");
      }
    else if (bytesRead == -1)
      {
	printf("NetPIPE: read: error encountered, errno=%d\n", errno);
	exit(401);
      }
}

void SendTime(ArgStruct *p, double *t)
{
    unsigned int ltime, ntime;

    /*
      Multiply the number of seconds by 1e6 to get time in microseconds
      and convert value to an unsigned 32-bit integer.
      */
    ltime = (unsigned int)(*t * 1.e6);

    /* Send time in network order */
    ntime = htonl(ltime);
    if (write(p->commfd, (char *)&ntime, sizeof(unsigned int)) < 0)
      {
	printf("NetPIPE: write failed in SendTime: errno=%d\n", errno);
	exit(301);
      }
}

void RecvTime(ArgStruct *p, double *t)
{
    unsigned int ltime, ntime;
    int bytesRead;

    bytesRead = readFully(p->commfd, (void *)&ntime, sizeof(unsigned int));
    if (bytesRead < 0)
      {
	printf("NetPIPE: read failed in RecvTime: errno=%d\n", errno);
	exit(302);
      }
    else if (bytesRead != sizeof(unsigned int))
      {
	fprintf(stderr, "NetPIPE: partial read in RecvTime of %d bytes\n",
		bytesRead);
	exit(303);
      }
    ltime = ntohl(ntime);

    /* Result is ltime (in microseconds) divided by 1.0e6 to get seconds */
    *t = (double)ltime / 1.0e6;
}

void SendRepeat(ArgStruct *p, int rpt)
{
  unsigned int lrpt, nrpt;

  lrpt = rpt;
  /* Send repeat count as an unsigned 32 bit integer in network order */
  nrpt = htonl(lrpt);
  if (write(p->commfd, (void *) &nrpt, sizeof(unsigned int)) < 0)
    {
      printf("NetPIPE: write failed in SendRepeat: errno=%d\n", errno);
      exit(304);
    }
}

void RecvRepeat(ArgStruct *p, int *rpt)
{
  unsigned int lrpt, nrpt;
  int bytesRead;

  bytesRead = readFully(p->commfd, (void *)&nrpt, sizeof(unsigned int));
  if (bytesRead < 0)
    {
      printf("NetPIPE: read failed in RecvRepeat: errno=%d\n", errno);
      exit(305);
    }
  else if (bytesRead != sizeof(unsigned int))
    {
      fprintf(stderr, "NetPIPE: partial read in RecvRepeat of %d bytes\n",
	      bytesRead);
      exit(306);
    }
  lrpt = ntohl(nrpt);

  *rpt = lrpt;
}

int Establish(ArgStruct *p)
{
 socklen_t clen;
 int one = 1;
 struct protoent *proto;

 clen = sizeof(p->prot.sin2);
 if (p->tr) {
   if (connect(p->commfd, (struct sockaddr *) &(p->prot.sin1),
	      sizeof(p->prot.sin1)) < 0) {
     printf("Client: Cannot Connect! errno=%d\n",errno);
     exit(-10);
   }
  }
  else {
    /* SERVER */
    listen(p->servicefd, 5);
    p->commfd = accept(p->servicefd, (struct sockaddr *) &(p->prot.sin2),
		       &clen);

    if (p->commfd < 0) {
      printf("Server: Accept Failed! errno=%d\n",errno);
      exit(-12);
    }

    /*
      Attempt to set TCP_NODELAY. TCP_NODELAY may or may not be propagated
      to accepted sockets.
     */
    if (!(proto = getprotobyname("tcp"))) {
      printf("unknown protocol!\n");
      exit(555);
    }

    if (setsockopt(p->commfd, proto->p_proto, TCP_NODELAY,
		  &one, sizeof(one)) < 0)
    {
      printf("setsockopt: TCP_NODELAY failed! errno=%d\n", errno);
      exit(556);
    }

    /* If requested, set the send and receive buffer sizes */
    if (p->prot.sndbufsz > 0)
    {
      printf("Send and Receive Buffers on accepted socket set to %d bytes\n",
	     p->prot.sndbufsz);
      if (setsockopt(p->commfd, SOL_SOCKET, SO_SNDBUF, &(p->prot.sndbufsz),
                                       sizeof(p->prot.sndbufsz)) < 0)
      {
	printf("setsockopt: SO_SNDBUF failed! errno=%d\n", errno);
	exit(556);
      }
      if (setsockopt(p->commfd, SOL_SOCKET, SO_RCVBUF, &(p->prot.rcvbufsz),
                                       sizeof(p->prot.rcvbufsz)) < 0)
      {
	printf("setsockopt: SO_RCVBUF failed! errno=%d\n", errno);
	exit(556);
      }
    }
  }
  return(0);
}

int CleanUp(ArgStruct *p)
{
 char *quit="QUIT";
 if (p->tr)
 {
     write(p->commfd,quit, 5);
     read(p->commfd, quit, 5);
     close(p->commfd);
 }
 else
 {
     read(p->commfd,quit, 5);
     write(p->commfd,quit,5);
     close(p->commfd);
     close(p->servicefd);
 }
 return(0);
}