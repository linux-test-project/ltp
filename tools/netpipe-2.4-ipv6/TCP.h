/*
  Define the protocol structure to be used by NetPIPE for TCP.

  2002/03/18 --- Modified for IPv6 - Robbie Williamson (robbiew@us.ibm.com)
  */

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

typedef struct protocolstruct ProtocolStruct;
struct protocolstruct
{
    struct sockaddr_in6     sin1,   /* socket structure #1              */
                            sin2;   /* socket structure #2              */
    int                     nodelay;  /* Flag for TCP nodelay           */
    struct addrinfo         *addr;    /* Address of host                */
    struct addrinfo         *server_addr; /* Address of server          */
    int                     sndbufsz, /* Size of TCP send buffer        */
                            rcvbufsz; /* Size of TCP receive buffer     */
};

