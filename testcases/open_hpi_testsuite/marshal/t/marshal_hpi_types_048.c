/*
 * Copyright (c) 2005 by IBM Corporation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     W. David Ashley <dashley@us.ibm.com>
 */

#include "marshal_hpi_types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


static int
cmp_text_buffer( SaHpiTextBufferT *d1, SaHpiTextBufferT *d2 )
{
  if ( d1->DataType != d2->DataType )
       return 0;

  if ( d1->Language != d2->Language )
       return 0;

  if ( d1->DataLength != d2->DataLength )
       return 0;

  return memcmp( d1->Data, d2->Data, d1->DataLength ) ? 0 : 1;
}


static int
cmp_userevent( SaHpiUserEventT *d1, SaHpiUserEventT *d2 )
{
  if ( !cmp_text_buffer( &d1->UserEventData, &d2->UserEventData ) )
       return 0;

  return 1;
}


static int
cmp_event( SaHpiEventT *d1, SaHpiEventT *d2 )
{
  if ( d1->Source != d2->Source )
       return 0;

  if ( d1->EventType != d2->EventType )
       return 0;

  if ( d1->Timestamp != d2->Timestamp )
       return 0;

  if ( !cmp_userevent( &d1->EventDataUnion.UserEvent, &d2->EventDataUnion.UserEvent ) )
       return 0;

  return 1;
}


static int
cmp_eventlogentry( SaHpiEventLogEntryT *d1, SaHpiEventLogEntryT *d2 )
{
  if ( d1->EntryId != d2->EntryId )
       return 0;

  if ( d1->Timestamp != d2->Timestamp )
       return 0;

  if ( !cmp_event( &d1->Event, &d2->Event ) )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiEventLogEntryT m_v1;
  tUint8 m_pad2;
  SaHpiEventLogEntryT m_v2;
  SaHpiEventLogEntryT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiEventLogEntryType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiEventLogEntryType ),
  dStructElement( cTest, m_v3   , SaHpiEventLogEntryType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                                         = 47,
    .m_v1.EntryId                                   = 1,
    .m_v1.Timestamp                                 = 1000,
    .m_v1.Event.Source                              = 1,
    .m_v1.Event.EventType                           = SAHPI_ET_USER,
    .m_v1.Event.Timestamp                           = 1000,
    .m_v1.Event.EventDataUnion.UserEvent.UserEventData.DataType   = SAHPI_TL_TYPE_BINARY,
    .m_v1.Event.EventDataUnion.UserEvent.UserEventData.Language   = SAHPI_LANG_TSONGA,
    .m_v1.Event.EventDataUnion.UserEvent.UserEventData.DataLength = 3,
    .m_v1.Event.EventDataUnion.UserEvent.UserEventData.Data       = "AB",
    .m_pad2                                         = 48,
    .m_v2.EntryId                                   = 2,
    .m_v2.Timestamp                                 = 2000,
    .m_v2.Event.Source                              = 2,
    .m_v2.Event.EventType                           = SAHPI_ET_USER,
    .m_v2.Event.Timestamp                           = 1200,
    .m_v2.Event.EventDataUnion.UserEvent.UserEventData.DataType   = SAHPI_TL_TYPE_BCDPLUS,
    .m_v2.Event.EventDataUnion.UserEvent.UserEventData.Language   = SAHPI_LANG_SANGRO,
    .m_v2.Event.EventDataUnion.UserEvent.UserEventData.DataLength = 21,
    .m_v2.Event.EventDataUnion.UserEvent.UserEventData.Data       = "12345678901234567890",
    .m_v3.EntryId                                   = 3,
    .m_v3.Timestamp                                 = 3000,
    .m_v3.Event.Source                              = 3,
    .m_v3.Event.EventType                           = SAHPI_ET_USER,
    .m_v3.Event.Timestamp                           = 1030,
    .m_v3.Event.EventDataUnion.UserEvent.UserEventData.DataType   = SAHPI_TL_TYPE_ASCII6,
    .m_v3.Event.EventDataUnion.UserEvent.UserEventData.Language   = SAHPI_LANG_TAJIK,
    .m_v3.Event.EventDataUnion.UserEvent.UserEventData.DataLength = 0,
    .m_v3.Event.EventDataUnion.UserEvent.UserEventData.Data       = "",
    .m_pad3                                         = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_eventlogentry( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_eventlogentry( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_eventlogentry( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
