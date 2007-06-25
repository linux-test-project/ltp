/*
 * ipmi_sensor_factors.h
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

#ifndef dIpmiSensorFactors_h
#define dIpmiSensorFactors_h


extern "C" {
#include "SaHpi.h"
}

#ifndef dIpmiSdr_h
#include "ipmi_sdr.h"
#endif


// analog data format
enum tIpmiAnalogeDataFormat
{
  eIpmiAnalogDataFormatUnsigned  = 0,
  eIpmiAnalogDataFormat1Compl    = 1,
  eIpmiAnalogDataFormat2Compl    = 2,
  eIpmiAnalogDataFormatNotAnalog = 3
};

const char *IpmiAnalogeDataFormatToString( tIpmiAnalogeDataFormat fmt );

// raw value linearization
enum tIpmiLinearization
{
  eIpmiLinearizationLinear    =  0,
  eIpmiLinearizationLn        =  1,
  eIpmiLinearizationLog10     =  2,
  eIpmiLinearizationLog2      =  3,
  eIpmiLinearizationE         =  4,
  eIpmiLinearizationExp10     =  5,
  eIpmiLinearizationExp2      =  6,
  eIpmiLinearization1OverX    =  7,
  eIpmiLinearizationSqr       =  8,
  eIpmiLinearizationCube      =  9,
  eIpmiLinearizationSqrt      = 10,
  eIpmiLinearization1OverCube = 11,
  eIpmiLinearizationNonlinear = 0x70
};

const char *IpmiLinearizationToString( tIpmiLinearization val );


class cIpmiSensorFactors
{
public:
  cIpmiSensorFactors();
  virtual ~cIpmiSensorFactors();

  virtual bool GetDataFromSdr( const cIpmiSdr *sdr );
  virtual bool Cmp( const cIpmiSensorFactors &sf ) const;

  tIpmiAnalogeDataFormat m_analog_data_format;
  tIpmiLinearization     m_linearization;
  bool                   m_is_non_linear;
  int                    m_m : 10;
  unsigned int           m_tolerance : 6;
  int                    m_b : 10;
  int                    m_r_exp : 4;
  unsigned int           m_accuracy_exp : 2;
  int                    m_accuracy : 10;
  int                    m_b_exp : 4;
  double                 m_accuracy_factor;

public:
  tIpmiAnalogeDataFormat AnalogDataFormat() const { return  m_analog_data_format; }
  tIpmiLinearization Linearization() const { return m_linearization; }
  int           M()        const { return m_m; }
  unsigned int  Tolerance() const { return m_tolerance; }
  int           B()        const { return m_b; }
  int           RExp()     const { return m_r_exp; }
  unsigned int  AccuracyExp() const { return m_accuracy_exp; }
  int           Accuracy() const { return m_accuracy; }
  double        AccuracyFactor() const { return m_accuracy_factor; }
  int           BExp()     const { return m_b_exp; }
  bool          IsNonLinear() const { return m_is_non_linear; }

  enum tIpmiRound
  {
      eRoundNormal,
      eRoundDown,
      eRoundUp
  };

  bool ConvertFromRaw( unsigned int val, double &result, bool is_hysteresis ) const;
  bool ConvertToRaw( tIpmiRound rounding, double val, unsigned int &result, bool is_hysteresis, bool swap_thresholds ) const;
};


#endif
