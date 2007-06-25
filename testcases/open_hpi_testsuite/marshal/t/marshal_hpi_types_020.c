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
cmp_ctrlrecanalog( SaHpiCtrlRecAnalogT *d1, SaHpiCtrlRecAnalogT *d2 )
{
  if ( d1->Min != d2->Min )
       return 0;

  if ( d1->Max != d2->Max )
       return 0;

  if ( d1->Default != d2->Default )
       return 0;

  return 1;
}


static int
cmp_ctrldefaultmode( SaHpiCtrlDefaultModeT *d1, SaHpiCtrlDefaultModeT *d2 )
{
  if ( d1->Mode != d2->Mode )
       return 0;

  if ( d1->ReadOnly != d2->ReadOnly )
       return 0;

  return 1;
}


static int
cmp_ctrlrec( SaHpiCtrlRecT *d1, SaHpiCtrlRecT *d2 )
{
  if ( d1->Num != d2->Num )
       return 0;

  if ( d1->OutputType != d2->OutputType )
       return 0;

  if ( d1->Type != d2->Type )
       return 0;

  if ( !cmp_ctrlrecanalog( &d1->TypeUnion.Analog, &d2->TypeUnion.Analog ) )
       return 0;

  if ( !cmp_ctrldefaultmode( &d1->DefaultMode, &d2->DefaultMode ) )
       return 0;

  if ( d1->WriteOnly != d2->WriteOnly )
       return 0;

  if ( d1->Oem != d2->Oem )
       return 0;

  return 1;
}


typedef struct
{
  tUint8 m_pad1;
  SaHpiCtrlRecT m_v1;
  tUint8 m_pad2;
  SaHpiCtrlRecT m_v2;
  SaHpiCtrlRecT m_v3;
  tUint8 m_pad3;
} cTest;

cMarshalType StructElements[] =
{
  dStructElement( cTest, m_pad1 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v1   , SaHpiCtrlRecType ),
  dStructElement( cTest, m_pad2 , Marshal_Uint8Type ),
  dStructElement( cTest, m_v2   , SaHpiCtrlRecType ),
  dStructElement( cTest, m_v3   , SaHpiCtrlRecType ),
  dStructElement( cTest, m_pad3 , Marshal_Uint8Type ),
  dStructElementEnd()
};

cMarshalType TestType = dStruct( cTest, StructElements );


int
main( int argc, char *argv[] )
{
  cTest value =
  {
    .m_pad1                        = 47,
    .m_v1.Num                      = 1,
    .m_v1.OutputType               = SAHPI_CTRL_GENERIC,
    .m_v1.Type                     = SAHPI_CTRL_TYPE_ANALOG,
    .m_v1.TypeUnion.Analog.Min     = -5,
    .m_v1.TypeUnion.Analog.Max     = 5,
    .m_v1.TypeUnion.Analog.Default = 0,
    .m_v1.DefaultMode.Mode         = SAHPI_CTRL_MODE_AUTO,
    .m_v1.DefaultMode.ReadOnly     = FALSE,
    .m_v1.WriteOnly                = FALSE,
    .m_v1.Oem                      = 0,
    .m_pad2                        = 48,
    .m_v2.Num                      = 1,
    .m_v2.OutputType               = SAHPI_CTRL_FAN_SPEED,
    .m_v2.Type                     = SAHPI_CTRL_TYPE_ANALOG,
    .m_v2.TypeUnion.Analog.Min     = -10,
    .m_v2.TypeUnion.Analog.Max     = 3,
    .m_v2.TypeUnion.Analog.Default = 1,
    .m_v2.DefaultMode.Mode         = SAHPI_CTRL_MODE_AUTO,
    .m_v2.DefaultMode.ReadOnly     = TRUE,
    .m_v2.WriteOnly                = FALSE,
    .m_v2.Oem                      = 0,
    .m_v3.Num                      = 1,
    .m_v3.OutputType               = SAHPI_CTRL_AUDIBLE,
    .m_v3.Type                     = SAHPI_CTRL_TYPE_ANALOG,
    .m_v3.TypeUnion.Analog.Min     = 0,
    .m_v3.TypeUnion.Analog.Max     = 20,
    .m_v3.TypeUnion.Analog.Default = 10,
    .m_v3.DefaultMode.Mode         = SAHPI_CTRL_MODE_MANUAL,
    .m_v3.DefaultMode.ReadOnly     = FALSE,
    .m_v3.WriteOnly                = TRUE,
    .m_v3.Oem                      = 0,
    .m_pad3                        = 49
  };

  unsigned char *buffer = (unsigned char *)malloc(sizeof(value));
  cTest          result;

  unsigned int s1 = Marshal( &TestType, &value, buffer );
  unsigned int s2 = Demarshal( MarshalByteOrder(), &TestType, &result, buffer );

  if ( s1 != s2 )
       return 1;

  if ( value.m_pad1 != result.m_pad1 )
       return 1;

  if ( !cmp_ctrlrec( &value.m_v1, &result.m_v1 ) )
       return 1;

  if ( value.m_pad2 != result.m_pad2 )
       return 1;

  if ( !cmp_ctrlrec( &value.m_v2, &result.m_v2 ) )
       return 1;

  if ( !cmp_ctrlrec( &value.m_v3, &result.m_v3 ) )
       return 1;

  if ( value.m_pad3 != result.m_pad3 )
       return 1;

  return 0;
}
