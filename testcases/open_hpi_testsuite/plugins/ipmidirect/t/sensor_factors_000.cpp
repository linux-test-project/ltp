/*
 * Test convertion for raw -> interpreted and interpreted -> raw
 *
 * Copyright (c) 2004 by FORCE Computers
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
 *
 */


#include "test.h"
#include "ipmi_sensor_factors.h"

#define dAnalogDataFormat eIpmiAnalogDataFormatUnsigned
#define dLinearization eIpmiLinearizationLinear
#define dM             136
#define dTolerance     9
#define dB             0
#define dAccuracy      15
#define dAccuracyExp   0
#define dRExp          -4
#define dBExp          0


static cIpmiSdr sdr = 
{
  0,
  1,
  5,
  eSdrTypeFullSensorRecord, 
  60,
  {
    0, 0, // record id
    0x51, // ipmi version
    0, // record type
    0, // length
    0, // owner id
    0, // lun
    0, // num
    0, // entity id
    0, // instance
    0, // initialization
    0, // capabilities
    0, // type
    0, // event reading type
    0, 0, // event mask
    0, 0, // event mask
    0, 0, // event mask
    dAnalogDataFormat << 6, // units 1
    0, // units 2
    0, // units 3
    eIpmiLinearizationLinear,
    dM & 0xff,
    ((dM >> 2) & 0xc0) | (dTolerance & 0x3f),
    dB & 0xff,
    ((dB >> 2) & 0xc0) | (dAccuracy & 0x3f),
    ((dAccuracy >> 2) & 0xf0) | ((dAccuracyExp << 2) & 0x0c),
    ((dRExp << 4) & 0xf0) | (dBExp & 0x0f ),
    0,
    0,
  }
};


int
main( int /*argc*/, char * /*argv*/[] )
{
  cIpmiSensorFactors *s = new cIpmiSensorFactors;

  Test( s->GetDataFromSdr( &sdr ) );

  cIpmiSensorFactors c;
  c.m_analog_data_format = dAnalogDataFormat;
  c.m_linearization      = dLinearization;
  c.m_m                  = dM;
  c.m_tolerance          = dTolerance;
  c.m_b                  = dB;
  c.m_r_exp              = dRExp;
  c.m_accuracy_exp       = dAccuracyExp;
  c.m_accuracy           = dAccuracy;
  c.m_b_exp              = dBExp;

  Test( s->Cmp( c ) );

  for( unsigned int i = 0; i < 256; i++ )
     {
       double d;
       unsigned int r;

       Test( s->ConvertFromRaw( i, d, false ) );
       Test( s->ConvertToRaw( cIpmiSensorFactors::eRoundNormal, d, r, false, false ) );
       Test( r == i );
     }

  delete s;

  return TestResult();
}
