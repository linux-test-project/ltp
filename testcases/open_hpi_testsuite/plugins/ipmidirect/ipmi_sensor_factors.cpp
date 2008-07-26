/*
 * ipmi_sensor_factors.cpp
 *
 * Copyright (c) 2004 by FORCE Computers
 * Copyright (c) 2005 by ESO Technologies.
 *
 * Note that this file is based on parts of OpenIPMI
 * written by Corey Minyard <minyard@mvista.com>
 * of MontaVista Software. Corey's code was helpful
 * and many thanks go to him. He gave the permission
 * to use this code in OpenHPI under BSD license.
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
 *     Pierre Sangouard  <psangouard@eso-tech.com>
 */


#include "ipmi_sensor_factors.h"
#include <math.h>


static const char *analoge_data_format[] =
{
  "Unsigned",
  "1Compl",
  "2Compl",
  "Analog"
};


const char *
IpmiAnalogeDataFormatToString( tIpmiAnalogeDataFormat fmt )
{
  if ( (int)fmt <= eIpmiAnalogDataFormatNotAnalog )
       return analoge_data_format[fmt];

  return "Invalid";
}


static const char *linearization_map[] =
{
  "Linear",
  "Ln",
  "Log10",
  "Log2",
  "E",
  "Exp10",
  "Exp2",
  "1OverX",
  "Sqr",
  "Cube",
  "Sqrt",
  "1OverCube"
};


const char *
IpmiLinearizationToString( tIpmiLinearization val )
{
  if ( val == eIpmiLinearizationNonlinear )
       return "NonLinear";

  if ( val <= eIpmiLinearization1OverCube )
       return linearization_map[val];

  return "Invalid";
}


cIpmiSensorFactors::cIpmiSensorFactors()
  : m_analog_data_format( eIpmiAnalogDataFormatUnsigned ),
    m_linearization( eIpmiLinearizationLinear ),
    m_is_non_linear( false ),
    m_m( 0 ),
    m_tolerance( 0 ),
    m_b( 0 ),
    m_r_exp( 0 ),
    m_accuracy_exp( 0 ),
    m_accuracy( 0 ),
    m_b_exp( 0 )
{
}


cIpmiSensorFactors::~cIpmiSensorFactors()
{
}


bool
cIpmiSensorFactors::GetDataFromSdr( const cIpmiSdr *sdr )
{
  m_analog_data_format = (tIpmiAnalogeDataFormat)((sdr->m_data[20] >> 6) & 3);
  m_linearization = (tIpmiLinearization)(sdr->m_data[23] & 0x7f);

  if ( m_linearization <= 11 )
     {
       m_m            = sdr->m_data[24] | ((sdr->m_data[25] & 0xc0) << 2);
       m_tolerance    = sdr->m_data[25] & 0x3f;
       m_b            = sdr->m_data[26] | ((sdr->m_data[27] & 0xc0) << 2);
       m_accuracy     = ((sdr->m_data[27] & 0x3f)
                            | ((sdr->m_data[28] & 0xf0) << 2));
       m_accuracy_exp = (sdr->m_data[28] >> 2) & 0x3;
       m_r_exp        = (sdr->m_data[29] >> 4) & 0xf;
       m_b_exp        = sdr->m_data[29] & 0xf;
       m_accuracy_factor = (m_accuracy * pow( 10.0, m_accuracy_exp)) / 100.0;
     }

  if ( m_linearization == eIpmiLinearizationLinear )
      m_is_non_linear = false;
  else
      m_is_non_linear = true;

  return true;
}


bool
cIpmiSensorFactors::Cmp( const cIpmiSensorFactors &sf ) const
{
  if ( m_analog_data_format != sf.m_analog_data_format )
       return false;

  if ( m_linearization != sf.m_linearization )
       return false;

  if ( m_linearization <= 11 )
     {
       if ( m_m            != sf.m_m            )
            return false;

       if ( m_tolerance    != sf.m_tolerance    )
            return false;

       if ( m_b            != sf.m_b            )
            return false;

       if ( m_accuracy     != sf.m_accuracy     )
            return false;

       if ( m_accuracy_exp != sf.m_accuracy_exp )
            return false;

       if ( m_r_exp        != sf.m_r_exp        )
            return false;

       if ( m_b_exp        != sf.m_b_exp        )
            return false;
     }

  return true;
}


static double
c_linear( double val )
{
  return val;
}


static double
c_exp10( double val )
{
  return pow( 10.0, val );
}


static double
c_exp2( double val )
{
  return pow( 2.0, val );
}


static double
c_1_over_x( double val )
{
  return 1.0 / val;
}


static double
c_sqr( double val )
{
  return pow( val, 2.0 );
}


static double
c_cube( double val )
{
  return pow( val, 3.0 );
}


static double
c_1_over_cube( double val )
{
  return 1.0 / pow( val, 3.0 );
}


typedef double (*linearizer)( double val );
static linearizer linearize[12] =
{
  c_linear,
  log,
  log10,
  log2,
  exp,
  c_exp10,
  c_exp2,
  c_1_over_x,
  c_sqr,
  c_cube,
  sqrt,
  c_1_over_cube
};


static int
sign_extend( int m, int bits )
{
    if ( m & (1 << (bits-1)) )
	return m | (-1 << bits);
    else
	return m & (~(-1 << bits));
}


bool
cIpmiSensorFactors::ConvertFromRaw( unsigned int val,
                                    double      &result,
                                    bool        is_hysteresis) const
{
  double m, b, b_exp, r_exp, fval;
  linearizer c_func;

  if ( m_linearization == eIpmiLinearizationNonlinear )
       c_func = c_linear;
  else if ( m_linearization <= 11 )
       c_func = linearize[m_linearization];
  else
       return false;

  val &= 0xff;

  m     = m_m;
  b     = m_b;
  r_exp = m_r_exp;
  b_exp = m_b_exp;

  if ( is_hysteresis == true )
  {
      if ( val == 0 )
      {
          result = 0;
          return true;
      }
      // For hysteresis : no offset + abs value
      b = 0;
      if ( m < 0 )
          m = -m;
  }

  switch( m_analog_data_format )
     {
       case eIpmiAnalogDataFormatUnsigned:
	    fval = val;
	    break;

       case eIpmiAnalogDataFormat1Compl:
	    val = sign_extend( val, 8 );
	    if ( val == 0xffffffff )
                 val += 1;

	    fval = val;
	    break;

       case eIpmiAnalogDataFormat2Compl:
	    fval = sign_extend( val, 8 );
	    break;

       default:
	    return false;
     }

  result = c_func( ((m * fval) + (b * pow(10, b_exp))) * pow(10, r_exp) );

  return true;
}


bool
cIpmiSensorFactors::ConvertToRaw( tIpmiRound    rounding,
                                  double        val,
                                  unsigned int &result,
                                  bool        is_hysteresis,
                                  bool        swap_thresholds ) const
{
  bool rv;
  bool swap;
  double cval;
  int    lowraw, highraw, raw, maxraw, minraw, next_raw;

  if (is_hysteresis == true)
    swap = false;
  else
    swap = swap_thresholds;

  switch( m_analog_data_format )
     {
       case eIpmiAnalogDataFormatUnsigned:
	    lowraw   = 0;
	    highraw  = 255;
	    minraw   = 0;
	    maxraw   = 255;
	    next_raw = 128;
	    break;

       case eIpmiAnalogDataFormat1Compl:
	    lowraw   = -127;
	    highraw  = 127;
	    minraw   = -127;
	    maxraw   = 127;
	    next_raw = 0;
	    break;

       case eIpmiAnalogDataFormat2Compl:
	    lowraw   = -128;
	    highraw  = 127;
	    minraw   = -128;
	    maxraw   = 127;
	    next_raw = 0;
	    break;

       default:
	    return false;
    }

  // We do a binary search for the right value.  Yuck, but I don't
  // have a better plan that will work with non-linear sensors.
  do
     {
       raw = next_raw;
       rv = ConvertFromRaw( raw, cval, is_hysteresis );

       if ( !rv )
	    return false;

       // If swap == true, when raw value increases
       // the corresponding interpreted value decreases
       // so we have to take that into account when searching
       if ((( swap == false) && ( cval < val ))
            || (( swap == true) && ( cval > val )))
          {
	    next_raw = ((highraw - raw) / 2) + raw;
	    lowraw = raw;
          } 
       else
          {
	    next_raw = ((raw - lowraw) / 2) + lowraw;
	    highraw = raw;
          }
     }
  while( raw != next_raw );

  // The above loop gets us to within 1 of what it should be, we
  // have to look at rounding to make the final decision.
  switch( rounding )
     {
       case eRoundNormal:
            // If swap == true, when raw value increases
            // the corresponding interpreted value decreases
            // so we have to take that into account when searching
            if ((( swap == false ) && ( val > cval ))
                || (( swap == true ) && ( val < cval )))
               {
                 if ( raw < maxraw )
                    {
                      double nval;

                      rv = ConvertFromRaw( raw + 1, nval, is_hysteresis );
                      if ( !rv )
                           return false;

                      nval = cval + ((nval - cval) / 2.0);
                      if ((( swap == false ) && ( val >= nval ))
                            || (( swap == true ) && ( val <= nval )))
                           raw++;
                    }
               }
            else
               {
                 if ( raw > minraw )
                    {
                      double pval;
                      rv = ConvertFromRaw( raw-1, pval, is_hysteresis );
                      if ( !rv )
                           return false;

                      pval = pval + ((cval - pval) / 2.0);
                      if ((( swap == false ) && ( val < pval ))
                            || (( swap == true ) && ( val > pval )))
                           raw--;
                    }
               }
	    break;

       case eRoundUp:
            // If swap == true, when raw value increases
            // the corresponding interpreted value decreases
            // so we have to take that into account when searching
            if ( swap == false )
               {
               if ((val > cval) && (raw < maxraw))
                    raw++;
               }
            else
               {
               if ((val < cval) && (raw < maxraw))
                    raw++;
               };
	    break;

       case eRoundDown:
            // If swap == true, when raw value increases
            // the corresponding interpreted value decreases
            // so we have to take that into account when searching
            if ( swap == false )
               {
                if ( ( val < cval) && (raw > minraw ) )
                    raw--;
               }
            else
               {
                if ( ( val > cval) && (raw > minraw ) )
                    raw--;
               };
	    break;
     }

  if ( m_analog_data_format == eIpmiAnalogDataFormat1Compl )
       if ( raw < 0 )
	    raw -= 1;

  result = raw & 0xff;
  return true;
}

