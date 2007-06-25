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
cmp_annunciatorrec( SaHpiAnnunciatorRecT *d1, SaHpiAnnunciatorRecT *d2 )
{
  if ( d1->AnnunciatorNum != d2->AnnunciatorNum )
       return 0;

  if ( d1->AnnunciatorType != d2->AnnunciatorType )
       return 0;

  if ( d1->ModeReadOnly != d2->ModeReadOnly )
       return 0;

  if ( d1->MaxConditions != d2->MaxConditions )
       return 0;

  if ( d1->Oem != d2->Oem )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiAnnunciatorRecT m_v1;
  tUint8 m_pad2;
  SaHpiAnnunciatorRecT m_v2;
  SaHpiAnnunciatorRecT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiAnnunciatorRecType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiAnnunciatorRecType ),
  dStructElement( cTest, m_v3   , SaHpiAnnunciatorRecType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                  = 47,
    .m_v1.AnnunciatorNum     = 1,
    .m_v1.AnnunciatorType    = SAHPI_ANNUNCIATOR_TYPE_LED,
    .m_v1.ModeReadOnly       = FALSE,
    .m_v1.MaxConditions      = 0,
    .m_v1.Oem                = 0,
    .m_pad2                  = 48,
    .m_v2.AnnunciatorNum     = 2,
    .m_v2.AnnunciatorType    = SAHPI_ANNUNCIATOR_TYPE_AUDIBLE,
    .m_v2.ModeReadOnly       = TRUE,
    .m_v2.MaxConditions      = 3,
    .m_v2.Oem                = 1000,
    .m_v3.AnnunciatorNum     = 100,
    .m_v3.AnnunciatorType    = SAHPI_ANNUNCIATOR_TYPE_MESSAGE,
    .m_v3.ModeReadOnly       = FALSE,
    .m_v3.MaxConditions      = 10,
    .m_v3.Oem                = 20,
    .m_pad3                  = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_annunciatorrec( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_annunciatorrec( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_annunciatorrec( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
