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


typedef struct
{
  tUint8 m_pad1;
  SaHpiResourceInfoT m_v1;
  tUint8 m_pad2;
  SaHpiResourceInfoT m_v2;
  SaHpiResourceInfoT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiResourceInfoType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiResourceInfoType ),
  dStructElement( cTest, m_v3   , SaHpiResourceInfoType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                                   = 47,
    .m_v1.ResourceRev                         = 1,
    .m_v1.SpecificVer                         = 1,
    .m_v1.DeviceSupport                       = 0,
    .m_v1.ManufacturerId                      = 10,
    .m_v1.ProductId                           = 10,
    .m_v1.FirmwareMajorRev                    = 3,
    .m_v1.FirmwareMinorRev                    = 3,
    .m_v1.AuxFirmwareRev                      = 0,
    .m_v1.Guid                                = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                                 10, 11, 12, 13, 14, 15},
    .m_pad2                                   = 48,
    .m_v2.ResourceRev                         = 1,
    .m_v2.SpecificVer                         = 1,
    .m_v2.DeviceSupport                       = 0,
    .m_v2.ManufacturerId                      = 10,
    .m_v2.ProductId                           = 10,
    .m_v2.FirmwareMajorRev                    = 3,
    .m_v2.FirmwareMinorRev                    = 3,
    .m_v2.AuxFirmwareRev                      = 0,
    .m_v2.Guid                                = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                                 10, 11, 12, 13, 14, 15},
    .m_v3.ResourceRev                         = 1,
    .m_v3.SpecificVer                         = 1,
    .m_v3.DeviceSupport                       = 0,
    .m_v3.ManufacturerId                      = 10,
    .m_v3.ProductId                           = 10,
    .m_v3.FirmwareMajorRev                    = 3,
    .m_v3.FirmwareMinorRev                    = 3,
    .m_v3.AuxFirmwareRev                      = 0,
    .m_v3.Guid                                = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                                 10, 11, 12, 13, 14, 15},
    .m_pad3                                   = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_resourceinfo( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_resourceinfo( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_resourceinfo( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
