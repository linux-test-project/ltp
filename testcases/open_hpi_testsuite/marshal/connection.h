/*
 * socket and message handling 
 *
 * Copyright (c) 2004 by FORCE Computers.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Thomas Kanngieser <thomas.kanngieser@fci.com>
 */

#ifndef dConnection_h
#define dConnection_h


#ifdef __cplusplus
extern "C" {
#endif


#include <netdb.h> 

// common errors codes
typedef enum
{
  eConnectionOk,
  eConnectionNew,
  eConnectionDuplicate,
  eConnectionError,
} tConnectionError;

// current version
#define dMhVersion 1

#define dMhGetVersion(flags) (((flags)>>4) & 0x7)

// message flags
#define dMhEndianBit 1

#define dMhRequest   0
#define dMhReply     2

#define dMhSingle    0
#define dMhFirst     1
#define dMhMiddle    2
#define dMhLast      3

#define dMhGetPackageType(flags) (((flags) >> 2) & 3)

// cMessageHeader::m_type
typedef enum
{
  eMhPing    = 1,
  eMhReset   = 2, // reset seq
  eMhMsg     = 3,
  eMhLast    = 4
} tMessageType;

int IsMessageType( tMessageType type );


// max message length including header
#define dMaxMessageLength 0xffff

typedef struct
{
  unsigned char m_type;
  unsigned char m_flags; // bit 0-3 flags, bit 4-7 version
  unsigned char m_seq;
  unsigned char m_seq_in; // != 0 for eMhReply to assign the request
  unsigned int  m_id;
  int           m_len;
} cMessageHeader;


#define IsReplyMsg(header) ((header)->m_flags & dMhReply)
#define IsRequestMsg(header) (!((header)->m_flags & dMhReply))


void MessageHeaderInit( cMessageHeader *header, tMessageType type,
                        unsigned char flags, unsigned char seq_in,
                        unsigned int id, unsigned int len );

// sequence number handling
typedef struct
{
  unsigned char  m_inbound_seq_num;
  unsigned char  m_outbound_seq_num;
  unsigned int   m_recv_msg_map;
} cConnectionSeq;


void             ConnectionSeqInit ( cConnectionSeq *cs );
unsigned char    ConnectionSeqGet  ( cConnectionSeq *cs ); // get next seq
tConnectionError ConnectionSeqCheck( cConnectionSeq *cs, unsigned char seq );


// write message
int ConnectionWriteMsg( int fd, struct sockaddr_in *addr, cMessageHeader *header, 
                        const void *msg );

// read message
int ConnectionReadMsg( int fd, struct sockaddr_in *rd_addr, cMessageHeader *header,
                       void *response );


// re-send of messages
struct sConnectionResendMsg;
typedef struct sConnectionResendMsg cConnectionResendMsg;

struct sConnectionResendMsg
{
  cMessageHeader m_msg;
  unsigned char *m_data;
};


typedef struct
{
  cConnectionSeq        m_seq;
  cConnectionResendMsg  m_msg[256];
  struct sockaddr_in    m_ip_addr;
  unsigned int          m_send_count; // number of messages send
  unsigned int          m_received_count; // number of messages received
} cConnectionResend;


void             ConnectionResendInit     ( cConnectionResend *rs,
                                           const struct sockaddr_in ip_addr );
void             ConnectionResendCleanup  ( cConnectionResend *rs );
int              ConnectionResendWriteMsg ( cConnectionResend *rs, int fd,
                                            cMessageHeader *header, const void *msg );
tConnectionError ConnectionResendHandleMsg( cConnectionResend *rs, int fd,
                                            cMessageHeader *header, const void *response );


// client

typedef struct
{
  cConnectionResend m_resend;
  int               m_fd;
} cClientConnection;


cClientConnection *ClientOpen    ( const char *host, int host_port );
void               ClientClose   ( cClientConnection *c );
int                ClientWriteMsg( cClientConnection *c,
                                   cMessageHeader *header,
                                   const void *request );
tConnectionError   ClientReadMsg ( cClientConnection *c,
                                   cMessageHeader *header,
                                   void *response );


// server
struct sServerConnection;
typedef struct sServerConnection cServerConnection;

typedef struct
{
  int                m_fd;
  cServerConnection *m_first;
} cServerSocket;


struct sServerConnection
{
  cServerConnection *m_next;
  cServerSocket     *m_socket;
  cConnectionResend  m_resend;
  void              *m_user_data;
};


cServerSocket   *ServerOpen         ( int port );
void             ServerClose        ( cServerSocket *s );
void             ServerConnectionRem( cServerConnection *c );
int              ServerWriteMsg     ( cServerConnection *c,
                                      cMessageHeader *header,
                                      const void *request );
tConnectionError ServerReadMsg      ( cServerSocket *c,
                                      cServerConnection **con,
                                      cMessageHeader *header,
                                      void *response );


#ifdef __cplusplus
}
#endif


#endif
