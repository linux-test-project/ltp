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
cmp_domaininfo( SaHpiDomainInfoT *d1, SaHpiDomainInfoT *d2 )
{
  int i;

  if ( d1->DomainId != d2->DomainId )
       return 0;

  if ( d1->DomainCapabilities != d2->DomainCapabilities )
       return 0;

  if ( d1->IsPeer != d2->IsPeer )
       return 0;

  if ( !cmp_text_buffer( &d1->DomainTag, &d2->DomainTag ) )
       return 0;

  if ( d1->DrtUpdateCount != d2->DrtUpdateCount )
       return 0;

  if ( d1->DrtUpdateTimestamp != d2->DrtUpdateTimestamp )
       return 0;

  if ( d1->RptUpdateCount != d2->RptUpdateCount )
       return 0;

  if ( d1->RptUpdateTimestamp != d2->RptUpdateTimestamp )
       return 0;

  if ( d1->DatUpdateCount != d2->DatUpdateCount )
       return 0;

  if ( d1->DatUpdateTimestamp != d2->DatUpdateTimestamp )
       return 0;

  if ( d1->ActiveAlarms != d2->ActiveAlarms )
       return 0;

  if ( d1->CriticalAlarms != d2->CriticalAlarms )
       return 0;

  if ( d1->MajorAlarms != d2->MajorAlarms )
       return 0;

  if ( d1->MinorAlarms != d2->MinorAlarms )
       return 0;

  if ( d1->DatUserAlarmLimit != d2->DatUserAlarmLimit )
       return 0;

  if ( d1->DatOverflow != d2->DatOverflow )
       return 0;

  for (i = 0; i < 16; i++) {
       if ( d1->Guid[i] != d2->Guid[i] )
            return 0;
  }

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiDomainInfoT m_v1;
  tUint8 m_pad2;
  SaHpiDomainInfoT m_v2;
  SaHpiDomainInfoT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiDomainInfoType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiDomainInfoType ),
  dStructElement( cTest, m_v3   , SaHpiDomainInfoType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                                      = 47,
    .m_v1.DomainId                               = 1,
    .m_v1.DomainCapabilities                     = 1,
    .m_v1.IsPeer                                 = TRUE,
    .m_v1.DomainTag.DataType                     = SAHPI_TL_TYPE_BINARY,
    .m_v1.DomainTag.Language                     = SAHPI_LANG_TSONGA,
    .m_v1.DomainTag.DataLength                   = 3,
    .m_v1.DomainTag.Data                         = "AB",
    .m_v1.DrtUpdateCount                         = 1,
    .m_v1.DrtUpdateTimestamp                     = 1006,
    .m_v1.RptUpdateCount                         = 1,
    .m_v1.RptUpdateTimestamp                     = 1050,
    .m_v1.DatUpdateCount                         = 1,
    .m_v1.DatUpdateTimestamp                     = 1400,
    .m_v1.ActiveAlarms                           = 1,
    .m_v1.CriticalAlarms                         = 1,
    .m_v1.MajorAlarms                            = 1,
    .m_v1.MinorAlarms                            = 1,
    .m_v1.DatUserAlarmLimit                      = 1,
    .m_v1.DatOverflow                            = FALSE,
    .m_v1.Guid                                   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                                    10, 11, 12, 13, 14, 15},
    .m_pad2                                      = 48,
    .m_v2.DomainId                               = 2,
    .m_v2.DomainCapabilities                     = 2,
    .m_v2.IsPeer                                 = TRUE,
    .m_v2.DomainTag.DataType                     = SAHPI_TL_TYPE_BINARY,
    .m_v2.DomainTag.Language                     = SAHPI_LANG_TSONGA,
    .m_v2.DomainTag.DataLength                   = 3,
    .m_v2.DomainTag.Data                         = "AB",
    .m_v2.DrtUpdateCount                         = 1,
    .m_v2.DrtUpdateTimestamp                     = 1100,
    .m_v2.RptUpdateCount                         = 1,
    .m_v2.RptUpdateTimestamp                     = 1020,
    .m_v2.DatUpdateCount                         = 1,
    .m_v2.DatUpdateTimestamp                     = 1003,
    .m_v2.ActiveAlarms                           = 0,
    .m_v2.CriticalAlarms                         = 1,
    .m_v2.MajorAlarms                            = 0,
    .m_v2.MinorAlarms                            = 1,
    .m_v2.DatUserAlarmLimit                      = 1,
    .m_v2.DatOverflow                            = TRUE,
    .m_v2.Guid                                   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                                    10, 11, 12, 13, 14, 15},
    .m_v3.DomainId                               = 3,
    .m_v3.DomainCapabilities                     = 3,
    .m_v3.IsPeer                                 = FALSE,
    .m_v3.DomainTag.DataType                     = SAHPI_TL_TYPE_BINARY,
    .m_v3.DomainTag.Language                     = SAHPI_LANG_TSONGA,
    .m_v3.DomainTag.DataLength                   = 3,
    .m_v3.DomainTag.Data                         = "AB",
    .m_v3.DrtUpdateCount                         = 2,
    .m_v3.DrtUpdateTimestamp                     = 1300,
    .m_v3.RptUpdateCount                         = 1,
    .m_v3.RptUpdateTimestamp                     = 1020,
    .m_v3.DatUpdateCount                         = 6,
    .m_v3.DatUpdateTimestamp                     = 1001,
    .m_v3.ActiveAlarms                           = 1,
    .m_v3.CriticalAlarms                         = 0,
    .m_v3.MajorAlarms                            = 0,
    .m_v3.MinorAlarms                            = 0,
    .m_v3.DatUserAlarmLimit                      = 1,
    .m_v3.DatOverflow                            = TRUE,
    .m_v3.Guid                                   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                                    10, 11, 12, 13, 14, 15},
    .m_pad3                                      = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_domaininfo( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_domaininfo( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_domaininfo( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
