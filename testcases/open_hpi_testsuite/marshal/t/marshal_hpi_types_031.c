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


static int
cmp_entity( SaHpiEntityT *d1, SaHpiEntityT *d2 )
{
  if ( d1->EntityType != d2->EntityType )
       return 0;

  if ( d1->EntityLocation != d2->EntityLocation )
       return 0;

  return 1;
}


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
cmp_rdr( SaHpiRdrT *d1, SaHpiRdrT *d2 )
{
  int i;

  if ( d1->RecordId != d2->RecordId )
       return 0;

  if ( d1->RdrType != d2->RdrType )
       return 0;

  for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++) {
      if (d1->Entity.Entry[i].EntityType == SAHPI_ENT_ROOT_VALUE &&
          d2->Entity.Entry[i].EntityType == SAHPI_ENT_ROOT_VALUE ) {
           break;
      }

      if ( !cmp_entity( &d1->Entity.Entry[i], &d2->Entity.Entry[i] ) )
           return 0;
  }

  if ( d1->IsFru != d2->IsFru )
       return 0;

  if ( !cmp_annunciatorrec( &d1->RdrTypeUnion.AnnunciatorRec,
                            &d2->RdrTypeUnion.AnnunciatorRec ) )
       return 0;

  if ( !cmp_text_buffer( &d1->IdString, &d2->IdString ) )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiRdrT m_v1;
  tUint8 m_pad2;
  SaHpiRdrT m_v2;
  SaHpiRdrT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiRdrType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiRdrType ),
  dStructElement( cTest, m_v3   , SaHpiRdrType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                                           = 47,
    .m_v1.RecordId                                    = 1,
    .m_v1.RdrType                                     = SAHPI_ANNUNCIATOR_RDR,
    .m_v1.Entity.Entry[0].EntityType                  = SAHPI_ENT_SYSTEM_BOARD,
    .m_v1.Entity.Entry[0].EntityLocation              = 1,
    .m_v1.Entity.Entry[1].EntityType                  = SAHPI_ENT_POWER_UNIT,
    .m_v1.Entity.Entry[1].EntityLocation              = 2,
    .m_v1.Entity.Entry[2].EntityType                  = SAHPI_ENT_ROOT,
    .m_v1.Entity.Entry[2].EntityLocation              = 0,
    .m_v1.IsFru                                       = FALSE,
    .m_v1.RdrTypeUnion.AnnunciatorRec.AnnunciatorNum  = 1,
    .m_v1.RdrTypeUnion.AnnunciatorRec.AnnunciatorType = SAHPI_ANNUNCIATOR_TYPE_LED,
    .m_v1.RdrTypeUnion.AnnunciatorRec.ModeReadOnly    = FALSE,
    .m_v1.RdrTypeUnion.AnnunciatorRec.MaxConditions   = 0,
    .m_v1.RdrTypeUnion.AnnunciatorRec.Oem             = 0,
    .m_v1.IdString.DataType                           = SAHPI_TL_TYPE_BINARY,
    .m_v1.IdString.Language                           = SAHPI_LANG_TSONGA,
    .m_v1.IdString.DataLength                         = 3,
    .m_v1.IdString.Data                               = "AB",
    .m_pad2                                           = 48,
    .m_v2.RecordId                                    = 1,
    .m_v2.RdrType                                     = SAHPI_ANNUNCIATOR_RDR,
    .m_v2.Entity.Entry[0].EntityType                  = SAHPI_ENT_SUB_CHASSIS,
    .m_v2.Entity.Entry[0].EntityLocation              = 1,
    .m_v2.Entity.Entry[1].EntityType                  = SAHPI_ENT_SYSTEM_BUS,
    .m_v2.Entity.Entry[1].EntityLocation              = 2,
    .m_v2.Entity.Entry[2].EntityType                  = SAHPI_ENT_COOLING_DEVICE,
    .m_v2.Entity.Entry[2].EntityLocation              = 3,
    .m_v2.Entity.Entry[3].EntityType                  = SAHPI_ENT_ROOT,
    .m_v2.Entity.Entry[3].EntityLocation              = 0,
    .m_v2.IsFru                                       = FALSE,
    .m_v2.RdrTypeUnion.AnnunciatorRec.AnnunciatorNum  = 2,
    .m_v2.RdrTypeUnion.AnnunciatorRec.AnnunciatorType = SAHPI_ANNUNCIATOR_TYPE_AUDIBLE,
    .m_v2.RdrTypeUnion.AnnunciatorRec.ModeReadOnly    = TRUE,
    .m_v2.RdrTypeUnion.AnnunciatorRec.MaxConditions   = 3,
    .m_v2.RdrTypeUnion.AnnunciatorRec.Oem             = 1000,
    .m_v2.IdString.DataType                           = SAHPI_TL_TYPE_BCDPLUS,
    .m_v2.IdString.Language                           = SAHPI_LANG_SANGRO,
    .m_v2.IdString.DataLength                         = 21,
    .m_v2.IdString.Data                               = "12345678901234567890",
    .m_v3.RecordId                                    = 1,
    .m_v3.RdrType                                     = SAHPI_ANNUNCIATOR_RDR,
    .m_v3.Entity.Entry[0].EntityType                  = SAHPI_ENT_PROCESSOR,
    .m_v3.Entity.Entry[0].EntityLocation              = 1,
    .m_v3.Entity.Entry[1].EntityType                  = SAHPI_ENT_ROOT,
    .m_v3.Entity.Entry[1].EntityLocation              = 0,
    .m_v3.IsFru                                       = TRUE,
    .m_v3.RdrTypeUnion.AnnunciatorRec.AnnunciatorNum  = 100,
    .m_v3.RdrTypeUnion.AnnunciatorRec.AnnunciatorType = SAHPI_ANNUNCIATOR_TYPE_MESSAGE,
    .m_v3.RdrTypeUnion.AnnunciatorRec.ModeReadOnly    = FALSE,
    .m_v3.RdrTypeUnion.AnnunciatorRec.MaxConditions   = 10,
    .m_v3.RdrTypeUnion.AnnunciatorRec.Oem             = 20,
    .m_v3.IdString.DataType                           = SAHPI_TL_TYPE_ASCII6,
    .m_v3.IdString.Language                           = SAHPI_LANG_TAJIK,
    .m_v3.IdString.DataLength                         = 0,
    .m_v3.IdString.Data                               = "",
    .m_pad3                                           = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_rdr( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_rdr( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_rdr( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
