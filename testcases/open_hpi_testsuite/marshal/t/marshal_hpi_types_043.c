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
cmp_resourceinfo( SaHpiResourceInfoT *d1, SaHpiResourceInfoT *d2 )
{
  int i;

  if ( d1->ResourceRev != d2->ResourceRev )
       return 0;

  if ( d1->SpecificVer != d2->SpecificVer )
       return 0;

  if ( d1->DeviceSupport != d2->DeviceSupport )
       return 0;

  if ( d1->ManufacturerId != d2->ManufacturerId )
       return 0;

  if ( d1->ProductId != d2->ProductId )
       return 0;

  if ( d1->FirmwareMajorRev != d2->FirmwareMajorRev )
       return 0;

  if ( d1->FirmwareMinorRev != d2->FirmwareMinorRev )
       return 0;

  if ( d1->AuxFirmwareRev != d2->AuxFirmwareRev )
       return 0;

  for (i = 0; i < 16; i++) {
       if ( d1->Guid[i] != d2->Guid[i] )
            return 0;
  }

  return 1;
}


static int
cmp_entities( SaHpiEntityT *d1, SaHpiEntityT *d2 )
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
cmp_rptentry( SaHpiRptEntryT *d1, SaHpiRptEntryT *d2 )
{
  if ( d1->EntryId != d2->EntryId )
       return 0;

  if ( d1->ResourceId != d2->ResourceId )
       return 0;

  if ( !cmp_resourceinfo( &d1->ResourceInfo, &d2->ResourceInfo ) )
       return 0;

  /* entity path is not compared here */

  if ( d1->ResourceCapabilities != d2->ResourceCapabilities )
       return 0;

  if ( d1->HotSwapCapabilities != d2->HotSwapCapabilities )
       return 0;

  if ( d1->ResourceSeverity != d2->ResourceSeverity )
       return 0;

  if ( d1->ResourceFailed != d2->ResourceFailed )
       return 0;

  if ( !cmp_text_buffer( &d1->ResourceTag, &d2->ResourceTag ) )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiRptEntryT m_v1;
  tUint8 m_pad2;
  SaHpiRptEntryT m_v2;
  SaHpiRptEntryT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiRptEntryType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiRptEntryType ),
  dStructElement( cTest, m_v3   , SaHpiRptEntryType ),
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
    .m_v1.EntryId                                = 1,
    .m_v1.ResourceId                             = 1,
    .m_v1.ResourceInfo.ResourceRev               = 1,
    .m_v1.ResourceInfo.SpecificVer               = 1,
    .m_v1.ResourceInfo.DeviceSupport             = 0,
    .m_v1.ResourceInfo.ManufacturerId            = 10,
    .m_v1.ResourceInfo.ProductId                 = 10,
    .m_v1.ResourceInfo.FirmwareMajorRev          = 3,
    .m_v1.ResourceInfo.FirmwareMinorRev          = 3,
    .m_v1.ResourceInfo.AuxFirmwareRev            = 0,
    .m_v1.ResourceInfo.Guid                      = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                                    10, 11, 12, 13, 14, 15},
    .m_v1.ResourceEntity.Entry[0].EntityType     = SAHPI_ENT_SYSTEM_BOARD,
    .m_v1.ResourceEntity.Entry[0].EntityLocation = 1,
    .m_v1.ResourceEntity.Entry[1].EntityType     = SAHPI_ENT_BATTERY,
    .m_v1.ResourceEntity.Entry[1].EntityLocation = 2,
    .m_v1.ResourceCapabilities                   = 1,
    .m_v1.HotSwapCapabilities                    = 1,
    .m_v1.ResourceSeverity                       = 1,
    .m_v1.ResourceFailed                         = TRUE,
    .m_v1.ResourceTag.DataType                   = SAHPI_TL_TYPE_BINARY,
    .m_v1.ResourceTag.Language                   = SAHPI_LANG_TSONGA,
    .m_v1.ResourceTag.DataLength                 = 3,
    .m_v1.ResourceTag.Data                       = "AB",
    .m_pad2                                      = 48,
    .m_v2.EntryId                                = 2,
    .m_v2.ResourceId                             = 2,
    .m_v2.ResourceInfo.ResourceRev               = 1,
    .m_v2.ResourceInfo.SpecificVer               = 1,
    .m_v2.ResourceInfo.DeviceSupport             = 0,
    .m_v2.ResourceInfo.ManufacturerId            = 10,
    .m_v2.ResourceInfo.ProductId                 = 10,
    .m_v2.ResourceInfo.FirmwareMajorRev          = 3,
    .m_v2.ResourceInfo.FirmwareMinorRev          = 3,
    .m_v2.ResourceInfo.AuxFirmwareRev            = 0,
    .m_v2.ResourceInfo.Guid                      = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                                    10, 11, 12, 13, 14, 15},
    .m_v2.ResourceEntity.Entry[0].EntityType     = SAHPI_ENT_POWER_MODULE,
    .m_v2.ResourceEntity.Entry[0].EntityLocation = 2,
    .m_v2.ResourceEntity.Entry[1].EntityType     = SAHPI_ENT_SYSTEM_BUS,
    .m_v2.ResourceEntity.Entry[1].EntityLocation = 3,
    .m_v2.ResourceCapabilities                   = 1,
    .m_v2.HotSwapCapabilities                    = 1,
    .m_v2.ResourceSeverity                       = 1,
    .m_v2.ResourceFailed                         = FALSE,
    .m_v2.ResourceTag.DataType                   = SAHPI_TL_TYPE_BCDPLUS,
    .m_v2.ResourceTag.Language                   = SAHPI_LANG_SANGRO,
    .m_v2.ResourceTag.DataLength                 = 21,
    .m_v2.ResourceTag.Data                       = "12345678901234567890",
    .m_v3.EntryId                                = 3,
    .m_v3.ResourceId                             = 3,
    .m_v3.ResourceInfo.ResourceRev               = 1,
    .m_v3.ResourceInfo.SpecificVer               = 1,
    .m_v3.ResourceInfo.DeviceSupport             = 0,
    .m_v3.ResourceInfo.ManufacturerId            = 10,
    .m_v3.ResourceInfo.ProductId                 = 10,
    .m_v3.ResourceInfo.FirmwareMajorRev          = 3,
    .m_v3.ResourceInfo.FirmwareMinorRev          = 3,
    .m_v3.ResourceInfo.AuxFirmwareRev            = 0,
    .m_v3.ResourceInfo.Guid                      = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                                    10, 11, 12, 13, 14, 15},
    .m_v3.ResourceEntity.Entry[0].EntityType     = SAHPI_ENT_SWITCH,
    .m_v3.ResourceEntity.Entry[0].EntityLocation = 3,
    .m_v3.ResourceEntity.Entry[1].EntityType     = SAHPI_ENT_SYSTEM_BLADE,
    .m_v3.ResourceEntity.Entry[1].EntityLocation = 4,
    .m_v3.ResourceEntity.Entry[2].EntityType     = SAHPI_ENT_RACK_MOUNTED_SERVER,
    .m_v3.ResourceEntity.Entry[2].EntityLocation = 5,
    .m_v3.ResourceCapabilities                   = 1,
    .m_v3.HotSwapCapabilities                    = 1,
    .m_v3.ResourceSeverity                       = 1,
    .m_v3.ResourceFailed                         = FALSE,
    .m_v3.ResourceTag.DataType                   = SAHPI_TL_TYPE_ASCII6,
    .m_v3.ResourceTag.Language                   = SAHPI_LANG_TAJIK,
    .m_v3.ResourceTag.DataLength                 = 0,
    .m_v3.ResourceTag.Data                       = "",
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

  if ( !cmp_rptentry( &value.m_v1, &result.m_v1 ) )
       return 1;
  if ( !cmp_entities( &value.m_v1.ResourceEntity.Entry[0], &result.m_v1.ResourceEntity.Entry[0] ) )
       return 1;
  if ( !cmp_entities( &value.m_v1.ResourceEntity.Entry[1], &result.m_v1.ResourceEntity.Entry[1] ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_rptentry( &value.m_v2, &result.m_v2 ) )
       return 1;
  if ( !cmp_entities( &value.m_v2.ResourceEntity.Entry[0], &result.m_v2.ResourceEntity.Entry[0] ) )
       return 1;
  if ( !cmp_entities( &value.m_v2.ResourceEntity.Entry[1], &result.m_v2.ResourceEntity.Entry[1] ) )
       return 1;

  if ( !cmp_rptentry( &value.m_v3, &result.m_v3 ) )
       return 1;
  if ( !cmp_entities( &value.m_v3.ResourceEntity.Entry[0], &result.m_v3.ResourceEntity.Entry[0] ) )
       return 1;
  if ( !cmp_entities( &value.m_v3.ResourceEntity.Entry[1], &result.m_v3.ResourceEntity.Entry[1] ) )
       return 1;
  if ( !cmp_entities( &value.m_v3.ResourceEntity.Entry[2], &result.m_v3.ResourceEntity.Entry[2] ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
