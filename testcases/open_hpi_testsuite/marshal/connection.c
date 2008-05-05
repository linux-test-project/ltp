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

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include "connection.h"
#include "marshal.h"


//#define dUdpTest 1


////////////////////////////////////////////////////////////
//                  message functions
////////////////////////////////////////////////////////////

int
IsMessageType( tMessageType type )
{
  if ( type >= eMhPing && type < eMhLast )
       return 1;

  return 0;
}


void
MessageHeaderInit( cMessageHeader *header, tMessageType type, unsigned char flags,
		   unsigned char seq_in, unsigned int id,
		   unsigned int len )
{
  memset( header, 0, sizeof( cMessageHeader ) );

  header->m_type    = type;
  header->m_flags   = flags;
  header->m_seq_in  = seq_in;
  header->m_id      = id;
  header->m_len     = len;
}


////////////////////////////////////////////////////////////
//                  sequence number handling
////////////////////////////////////////////////////////////

void
ConnectionSeqInit( cConnectionSeq *cs )
{
  cs->m_inbound_seq_num  = 1;
  cs->m_outbound_seq_num = 0;
  cs->m_recv_msg_map     = 0;
}


unsigned char
ConnectionSeqGet( cConnectionSeq *cs )
{
  cs->m_outbound_seq_num = (cs->m_outbound_seq_num + 1) % 256;

  if ( cs->m_outbound_seq_num == 0 )
       cs->m_outbound_seq_num = 1;

  return cs->m_outbound_seq_num;
}


tConnectionError
ConnectionSeqCheck( cConnectionSeq *cs, unsigned char seq )
{
  unsigned char d = seq - cs->m_inbound_seq_num;

  if ( d < 16 )
     {
       // It's after the current sequence number, but within 15.
       unsigned int bit = 1 << (d + 16);

       if ( cs->m_recv_msg_map & bit )
            return eConnectionDuplicate;

       // move the sequence number forward.
       cs->m_recv_msg_map >>= d;

       cs->m_inbound_seq_num = seq;
       cs->m_recv_msg_map |= 1 << 16;

       return eConnectionOk;
     }

  d = cs->m_inbound_seq_num - seq;

  if ( d <= 16 )
     {
       // It's before the current sequence number, but within 16.
       unsigned int bit = 1 << d;

       if ( cs->m_recv_msg_map & bit )
	    return eConnectionDuplicate;

       // move the sequence number backward.
       cs->m_recv_msg_map <<= d;

       cs->m_inbound_seq_num = seq;
       cs->m_recv_msg_map |= 1 << 16;

       return eConnectionOk;
     }

  // It's outside the current sequence number range, discard
  // the msg.
  return eConnectionError;
}


////////////////////////////////////////////////////////////
//                  read and write messages
////////////////////////////////////////////////////////////

int
ConnectionWriteMsg( int fd, struct sockaddr_in *addr, cMessageHeader *header, 
		    const void *msg )
{
  assert( IsMessageType( header->m_type ) );

  assert(    (header->m_type == eMhPing && header->m_seq == 0)
          || (header->m_type != eMhPing && header->m_seq ) );

  // set version
  header->m_flags &= 0x0f;
  header->m_flags |= dMhVersion << 4;

  // set endian
  header->m_flags &= ~dMhEndianBit;
  header->m_flags |= MarshalByteOrder();

  int l = sizeof( cMessageHeader ) + header->m_len;

  if ( l > dMaxMessageLength )
     {
       fprintf( stderr, "message too big: %d !\n", l );

       assert( 0 );
       return -1;
     }

  unsigned char data[dMaxMessageLength];

  memcpy( data, header, sizeof( cMessageHeader ) );

  if ( header->m_len )
     {
       assert( msg );
       memcpy( data + sizeof( cMessageHeader ), msg, header->m_len );
     }

#ifdef dUdpTest
  // simple test for lost upd messages:
  //       remove 50% of all messages
  if ( rand() < RAND_MAX / 2 )
       return 0;
#endif

  int rv = sendto( fd, data, l, 0,
                   (struct sockaddr *)addr,
                   sizeof( struct sockaddr_in ) );

  if ( rv != l )
     {
       fprintf( stderr, "write error: %s !\n", strerror( errno ) );
       return -1;
     }

  return 0;
}


int
ConnectionReadMsg( int fd, struct sockaddr_in *rd_addr, cMessageHeader *header,
                   void *response )
{
  socklen_t from_len = sizeof( struct sockaddr_in );
  unsigned char data[dMaxMessageLength];

  int len = recvfrom( fd, data, dMaxMessageLength, 0, (struct sockaddr *)rd_addr, &from_len );

  if ( len < sizeof( cMessageHeader ) )
     {
       fprintf( stderr, "drop message because header too small !\n" );
       return -1;
     }

  memcpy( header, data, sizeof( cMessageHeader ) );

  if ( (header->m_flags >> 4) != dMhVersion )
     {
       fprintf( stderr, "drop message because of wrong header version: %d, expect %d !\n",
                (header->m_flags>>4), dMhVersion );
       return -1;
     }

  if ( !IsMessageType( header->m_type ) )
     {
       fprintf( stderr, "drop message because of wrong message type: %d !\n",
                header->m_type );
       return -1;
     }

  // swap id and len if nessesary
  if ( (header->m_flags & dMhEndianBit) != MarshalByteOrder() )
     {
       header->m_id  = GUINT32_SWAP_LE_BE( header->m_id );
       header->m_len = GUINT32_SWAP_LE_BE( header->m_len );
     }

  if ( header->m_len > dMaxMessageLength - sizeof( cMessageHeader ) )
     {
       fprintf( stderr, "drop message because too big message: %d !\n",
                header->m_len );
       return -1;
     }

  if ( header->m_len )
     {
       assert( response );
       memcpy( response, data + sizeof( cMessageHeader ), header->m_len );
     }

  return 0;
}


////////////////////////////////////////////////////////////
//                  resend messages
////////////////////////////////////////////////////////////

void
ConnectionResendInit( cConnectionResend *rs, const struct sockaddr_in ip_addr )
{
  assert( rs );

  memset( rs, 0, sizeof( cConnectionResend ) );

  ConnectionSeqInit( &rs->m_seq );

  rs->m_ip_addr = ip_addr;
  rs->m_send_count     = 0;
  rs->m_received_count = 0;
}


void
ConnectionResendCleanup( cConnectionResend *rs )
{
  int i;

  for( i = 0; i < 256; i++ )
       if ( rs->m_msg[i].m_data )
          {
            free( rs->m_msg[i].m_data );
            rs->m_msg[i].m_data = 0;
          }

  ConnectionResendInit( rs, rs->m_ip_addr );
}


int
ConnectionResendWriteMsg( cConnectionResend *rs, int fd,
                          cMessageHeader *header, const void *msg )
{
  if ( header->m_type != eMhPing )
     {
       assert( header->m_len <= dMaxMessageLength );
       header->m_seq = ConnectionSeqGet( &rs->m_seq );

       assert( header->m_seq );

       if ( rs->m_send_count < 255 )
            assert( header->m_seq == rs->m_send_count + 1 );

       if ( IsReplyMsg( header ) )
          {
            unsigned char seq = header->m_seq_in;
            assert( seq );

            cConnectionResendMsg *m = &rs->m_msg[seq];

            if ( m->m_data )
                 free( m->m_data );

            memcpy( &m->m_msg, header, sizeof( cMessageHeader ) );

            if ( header->m_len )
               {
                 assert( msg );
                 m->m_data = malloc( header->m_len );
                 memcpy( m->m_data, msg, header->m_len );
               }
            else
               {
                 assert( msg == 0 );
                 m->m_data = 0;
               }

            assert( rs->m_msg[seq].m_msg.m_seq == header->m_seq );

            //fprintf( stderr, "write msg: %d, %d.\n", seq, header->m_seq );            
          }

       rs->m_send_count += 1;

       if ( IsReplyMsg( header ) && header->m_type == eMhReset )
            ConnectionResendCleanup( rs );
     }
  else
     {
       header->m_seq = 0;
       header->m_seq_in = 0;

       assert( header->m_len == 0 );
       assert( msg == 0 );
     }

  return ConnectionWriteMsg( fd, &rs->m_ip_addr, header, msg );
}


tConnectionError
ConnectionResendHandleMsg( cConnectionResend *rs, int fd,
                           cMessageHeader *header, const void *response )
{
  if ( header->m_type == eMhPing )
     {
       assert( header->m_len == 0 );
       assert( header->m_seq == 0 );
       assert( header->m_seq_in == 0 );

       return eConnectionOk;
     }
  else
       rs->m_received_count += 1;

  //fprintf( stderr, "read msg: %d.\n", header->m_seq );

  if ( header->m_seq == 0 )
     {
       fprintf( stderr, "read invalid message with seq 0 !\n" );
       return eConnectionError;
     }

  tConnectionError rv = ConnectionSeqCheck( &rs->m_seq, header->m_seq );

  switch( header->m_type )
     {
       case eMhReset:
            if ( IsReplyMsg(header) )
                 ConnectionResendCleanup( rs );

            return eConnectionOk;

       case eMhMsg:
            if ( IsRequestMsg( header ) )
               {
                 if ( rv == eConnectionDuplicate )
                    {
                      cConnectionResendMsg *m = &rs->m_msg[header->m_seq];

                      //fprintf( stderr, "re-send reply %d %d.\n",
                      //         m->m_msg.m_seq_in, m->m_msg.m_seq );

                      // check for reply
                      if ( m->m_msg.m_seq )
                           // resend reply
                           ConnectionWriteMsg( fd, &rs->m_ip_addr, &m->m_msg, m->m_data );
                    }
               }

            break;

       default:
            fprintf( stderr, "drop message because of unknown type %d !\n", header->m_type );
            return eConnectionError;
     }

  if ( IsRequestMsg( header ) && rv == eConnectionOk )
     {
       //fprintf( stderr, "remove old msg: %d %d.\n", header->m_seq,
       //         rs->m_msg[header->m_seq].m_msg.m_seq_in );

       cConnectionResendMsg *m = &rs->m_msg[header->m_seq];

       if ( m->m_data )
          {
            free( m->m_data );
            m->m_data = 0;
          }

       m->m_msg.m_seq = 0;
     }

  return rv;
}


////////////////////////////////////////////////////////////
//                  client functions
////////////////////////////////////////////////////////////

cClientConnection *
ClientOpen( const char *host, int host_port )
{
  // host addr
  struct hostent *ent = gethostbyname( host );

  if ( !ent )
     {
       fprintf( stderr, "cannot resolve %s !\n", host );
       return 0;
     }

  struct in_addr host_addr;
  memcpy( &host_addr, ent->h_addr_list[0], ent->h_length );

  // create socket
  int fd = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

  if ( fd == -1 )
     {
       fprintf( stderr, "cannot create socket: %s\n", strerror( errno ) );
       return 0;
     }

  int curr_port = 8000;
  int rv;

  do
     {
       curr_port++;

       struct sockaddr_in addr;

       addr.sin_family = AF_INET;
       addr.sin_port = htons( curr_port );
       addr.sin_addr.s_addr = INADDR_ANY;

       rv = bind( fd, (struct sockaddr *)&addr, sizeof( addr ) );
     }
  while( curr_port < 8100 && rv == -1 );

  if ( rv == -1 )
     {
       fprintf( stderr, "cannot bind: %s\n", strerror( errno ) );

       close( fd );

       return 0;
     }

  cClientConnection *c = malloc( sizeof( cClientConnection ) );

  struct sockaddr_in addr;
  addr.sin_addr   = host_addr;
  addr.sin_family = AF_INET;
  addr.sin_port   = htons( host_port );

  ConnectionResendInit( &c->m_resend, addr );
  c->m_fd = fd;

  return c;
}


void
ClientClose( cClientConnection *c )
{
  if ( c->m_fd )
       close( c->m_fd );

  ConnectionResendCleanup( &c->m_resend );  
  free( c );
}


int
ClientWriteMsg( cClientConnection *c,
                cMessageHeader *header, const void *request )
{
  return ConnectionResendWriteMsg( &c->m_resend, c->m_fd, header, request );
}


tConnectionError
ClientReadMsg( cClientConnection *c,
               cMessageHeader *header, void *request )
{
  struct sockaddr_in addr;

  int rv = ConnectionReadMsg( c->m_fd, &addr, header, request );

  if ( rv )
       return eConnectionError;

  // check addr
  struct sockaddr_in *ipaddr = (struct sockaddr_in *)(void *)&addr;

  if (    (ipaddr->sin_port        != c->m_resend.m_ip_addr.sin_port)
       || (ipaddr->sin_addr.s_addr != c->m_resend.m_ip_addr.sin_addr.s_addr) )
     {
       fprintf( stderr, "dropped message due to invalid ip !\n" );
       return eConnectionError;
     }

  return ConnectionResendHandleMsg( &c->m_resend, c->m_fd, header, request );
}


////////////////////////////////////////////////////////////
//                  server functions
////////////////////////////////////////////////////////////

cServerSocket *
ServerOpen( int port )
{
  int fd = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

  if ( fd == -1 )
     {
       fprintf( stderr, "cannot open udp server socket: %s !\n",
			    strerror( errno ) );
       return 0;
     }

  struct sockaddr_in addr;
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons( port );
  addr.sin_addr.s_addr = INADDR_ANY;

  int rv = bind( fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in) );

  if ( rv == -1 )
     {
       fprintf( stderr, "cannot bind socket to port: %s !\n",
                strerror( errno ) );

       close( fd );

       return 0;
     }

  cServerSocket *s = malloc( sizeof( cServerSocket ) );
  s->m_fd    = fd;
  s->m_first = 0;

  return s;
}


void
ServerClose( cServerSocket *s )
{
  while( s->m_first )
       ServerConnectionRem( s->m_first );

  if ( s->m_fd )
       close( s->m_fd );

  free( s );
}


static void
ServerConnectionAdd( cServerSocket *s,
                     cServerConnection *c )
{
  c->m_next   = s->m_first;
  c->m_socket = s;
  s->m_first  = c;
}


void
ServerConnectionRem( cServerConnection *c )
  
{
  cServerSocket *s = c->m_socket;
  cServerConnection *prev = 0;
  cServerConnection *current = s->m_first;

  while( current )
     {
       if ( current == c )
          {
            // found 
            if ( prev == 0 )
                 s->m_first   = c->m_next;
            else
                 prev->m_next = c->m_next;

            ConnectionResendCleanup( &c->m_resend );
            free( c );

            return;
          }

       prev = current;
       current = current->m_next;
     }

  // connection not found
  assert( 0 );
}


int
ServerWriteMsg( cServerConnection *c,
                cMessageHeader *header,
                const void *request )
{
  return ConnectionResendWriteMsg( &c->m_resend, c->m_socket->m_fd, header, request );
}


tConnectionError
ServerReadMsg( cServerSocket *s, cServerConnection **con,
               cMessageHeader *header,
               void *response )
{
  struct sockaddr_in addr;

  int r = ConnectionReadMsg( s->m_fd, &addr, header,
                             response );

  if ( r )
       return eConnectionError;

  // find connection
  cServerConnection  *c = s->m_first;
  *con = 0;

  while( c )
     {
       if (    (addr.sin_port        == c->m_resend.m_ip_addr.sin_port)
            && (addr.sin_addr.s_addr == c->m_resend.m_ip_addr.sin_addr.s_addr) )
          {
            *con = c;
            break;
          }

       c = c->m_next;
     }

  if ( *con == 0 )
     {
       c = malloc( sizeof( cServerConnection ) );
       c->m_next    = 0;
       c->m_socket  = s;
       ConnectionResendInit( &c->m_resend, addr );
       c->m_user_data = 0;

       ServerConnectionAdd( s, c );

       *con = c;

       tConnectionError r = ConnectionResendHandleMsg( &c->m_resend, s->m_fd, header, response );

       if ( r != eConnectionOk )
          {
            assert( 0 );
            return r;
          }

       return eConnectionNew;
     }

  return ConnectionResendHandleMsg( &c->m_resend, s->m_fd, header, response );
}
