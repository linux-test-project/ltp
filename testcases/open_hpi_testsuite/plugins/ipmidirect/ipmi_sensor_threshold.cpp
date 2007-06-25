/*
 * ipmi_sensor_threshold.cpp
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

#include "ipmi_sensor_threshold.h"
#include "ipmi_log.h"
#include "ipmi_domain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>


static const char *hysteresis_support_types[] =
{
    "None",
    "Readable",
    "Settable",
    "Fixed",
};


const char *
IpmiHysteresisSupportToString( tIpmiHysteresisSupport val )
{
  if ( val > eIpmiHysteresisSupportFixed )
       return "Invalid";
 
  return hysteresis_support_types[val];
}


static const char *threshold_access_support_types[] =
{
    "None",
    "Readable",
    "Settable",
    "Fixed",
};


const char *
IpmiThresholdAccessSupportToString( tIpmiThresholdAccessSuport val )
{
  if ( val > eIpmiThresholdAccessSupportFixed )
       return "Invalid";

  return threshold_access_support_types[val];
}


static void
AddOrStr( char *str, const char *s )
{
  if ( *str )
       strcat( str, " | " );

  strcat( str, s );
}


void
IpmiThresholdEventMaskToString( unsigned short em, char *str )
{
  *str = 0;

  if ( em & dIpmiEventLowerNonCriticalLow )
       AddOrStr( str, "LowerNonCriticalLow" );

  if ( em & dIpmiEventLowerNonCriticalHigh )
       AddOrStr( str, "LowerNonCriticalHigh" );

  if ( em & dIpmiEventLowerCriticalLow )
       AddOrStr( str, "LowerCriticalLow" );

  if ( em & dIpmiEventLowerCriticalHigh )
       AddOrStr( str, "LowerCriticalHigh" );

  if ( em & dIpmiEventLowerNonRecoverableLow )
       AddOrStr( str, "LowerNonRecoverableLow" );

  if ( em & dIpmiEventLowerNonRecoverableHigh )
       AddOrStr( str, "LowerNonRecoverableHigh" );

  if ( em & dIpmiEventUpperNonCriticalLow )
       AddOrStr( str, "UpperNonCriticalLow" );

  if ( em & dIpmiEventUpperCriticalHigh )
       AddOrStr( str, "UpperCriticalHigh" );

  if ( em & dIpmiEventUpperNonRecoverableLow )
       AddOrStr( str, "UpperNonRecoverableLow" );

  if ( em & dIpmiEventUpperNonRecoverableHigh )
       AddOrStr( str, "UpperNonRecoverableHigh" );
}


static const char *rate_unit[] =
{
  "None",
  "Us"
  "Ms"
  "Second",
  "Minute",
  "Hour",
  "Day"
};

#define dNumRateUnit (sizeof(rate_unit)/sizeof(char *))

const char *
IpmiRateUnitToString( tIpmiRateUnit val )
{
  if ( val > (int)dNumRateUnit )
       return "Invalid";

  return rate_unit[val];  
}


const char *modifier_unit_map[] =
{
  "None",
  "BasicDivModifier",
  "BasicMulModifier"
};

#define dNumModifierUnit (sizeof(modifier_unit_map)/sizeof(char *))

const char *
IpmiModifierUnitToString( tIpmiModifierUnit unit )
{
  if ( unit > (int)dNumModifierUnit )
       return "Invalid";

  return modifier_unit_map[unit];
}


static const char *unit_types[] =
{
  "Unspecified",
  "C",
  "F",
  "K",
  "Volts",
  "Amps",
  "Watts",
  "Joules",
  "Coulombs",
  "VA",
  "Nits",
  "Lumens",
  "Lux",
  "Candela",
  "Kpa",
  "Psi",
  "Newtons",
  "Cfm",
  "Rpm",
  "Hz",
  "Useconds",
  "Mseconds",
  "Seconds",
  "Minute",
  "Hour",
  "Day",
  "Week",
  "Mil",
  "Inches",
  "Feet",
  "CubicInchs",
  "CubicFeet",
  "Millimeters",
  "Centimeters",
  "Meters",
  "CubicCentimeters"
  "cubic meters",
  "Liters",
  "FluidOunces",
  "Radians",
  "Seradians",
  "Revolutions",
  "Cycles",
  "Gravities",
  "Ounces",
  "Pounds",
  "FootPounds",
  "OunceInches",
  "Gauss",
  "Gilberts",
  "Henries",
  "Mhenries",
  "Farads",
  "Ufarads",
  "Ohms",
  "Siemens",
  "Moles",
  "Becquerels",
  "Ppm",
  "Unspecified",
  "Decibels",
  "Dba",
  "Dbc",
  "Grays",
  "Sieverts",
  "ColorTempDegK",
  "Bits",
  "Kbits",
  "Mbits",
  "Gbits",
  "Bytes",
  "Kbytes",
  "Mbytes",
  "Gbytes",
  "Words",
  "Dwords",
  "Qwords",
  "Lines",
  "Hits",
  "Misses",
  "Retries",
  "Resets",
  "Overruns",
  "Underruns",
  "Collisions",
  "Packets",
  "Messages",
  "Characters",
  "Errors",
  "CorrectableErrors",
  "UncorrectableErrors"
};

#define dNumUnitTypes (sizeof(unit_types)/sizeof(char *))


const char *
IpmiUnitTypeToString( tIpmiUnitType val )
{
  if ( val > (int)dNumUnitTypes )
       return "invalid";

  return unit_types[val];
}


cIpmiSensorThreshold::cIpmiSensorThreshold( cIpmiMc *mc )
  : cIpmiSensor( mc ), 
    m_sensor_init_thresholds( false ),
    m_sensor_init_hysteresis( false ),
    m_hysteresis_support( eIpmiHysteresisSupportNone ),
    m_threshold_access( eIpmiThresholdAccessSupportNone ),
    m_threshold_readable( 0 ),
    m_threshold_settable( 0 ),
    m_rate_unit( eIpmRateUnitNone ),
    m_modifier_unit_use( eIpmiModifierUnitNone ),
    m_percentage( false ),
    m_base_unit( eIpmiUnitTypeUnspecified ),
    m_modifier_unit( eIpmiUnitTypeUnspecified ),
    m_normal_min_specified( false ),
    m_normal_max_specified( false ),
    m_nominal_reading_specified( false ),
    m_nominal_reading( 0 ),
    m_normal_max( 0 ),
    m_normal_min( 0 ),
    m_sensor_max( 0 ),
    m_sensor_min( 0 ),
    m_upper_non_recoverable_threshold( 0 ),
    m_upper_critical_threshold( 0 ),
    m_upper_non_critical_threshold( 0 ),
    m_lower_non_recoverable_threshold( 0 ),
    m_lower_critical_threshold( 0 ),
    m_lower_non_critical_threshold( 0 ),
    m_positive_going_threshold_hysteresis( 0 ),
    m_negative_going_threshold_hysteresis( 0 ),
    m_sensor_factors( 0 )
{
}


cIpmiSensorThreshold::~cIpmiSensorThreshold()
{
  if ( m_sensor_factors )
       delete m_sensor_factors;
}


bool
cIpmiSensorThreshold::GetDataFromSdr( cIpmiMc *mc, cIpmiSdr *sdr )
{
  if ( !cIpmiSensor::GetDataFromSdr( mc, sdr ) )
       return false;

  m_sensor_init_thresholds  = (sdr->m_data[10] >> 4) & 1;
  m_sensor_init_hysteresis  = (sdr->m_data[10] >> 3) & 1;

  m_hysteresis_support      = (tIpmiHysteresisSupport)((sdr->m_data[11] >> 4) & 3);
  m_threshold_access        = (tIpmiThresholdAccessSuport)((sdr->m_data[11] >> 2) & 3);

  // assertion
  unsigned int val = IpmiGetUint16( sdr->m_data + 14 );

  m_assertion_event_mask = val & dIpmiEventMask;
  m_current_hpi_assert_mask = GetEventMask( m_assertion_event_mask );
  m_hpi_assert_mask = m_current_hpi_assert_mask;
  m_reading_mask = (val >> 12) & 7;
  
  // deassertion
  val = IpmiGetUint16( sdr->m_data + 16 );

  m_deassertion_event_mask = val & dIpmiEventMask;
  m_current_hpi_deassert_mask = GetEventMask( m_deassertion_event_mask );
  m_hpi_deassert_mask = m_current_hpi_deassert_mask;

  m_reading_mask |= ((val >> 12) & 7 ) << 3;

  val = IpmiGetUint16( sdr->m_data + 18 );
  m_threshold_readable = val & 0x3f;
  m_threshold_settable  = (val >> 8) & 0x3f;

  m_rate_unit          = (tIpmiRateUnit)((sdr->m_data[20] >> 3) & 7);
  m_modifier_unit_use  = (tIpmiModifierUnit)((sdr->m_data[20] >> 1) & 3);
  m_percentage         = sdr->m_data[20] & 1;
  m_base_unit          = (tIpmiUnitType)sdr->m_data[21];
  m_modifier_unit      = (tIpmiUnitType)sdr->m_data[22];

  m_sensor_factors = CreateSensorFactors( mc, sdr );

  if ( !m_sensor_factors )
       return false;

  m_normal_min_specified = (sdr->m_data[30] >> 2) & 1;
  m_normal_max_specified = (sdr->m_data[30] >> 1) & 1;
  m_nominal_reading_specified = sdr->m_data[30] & 1;
  m_nominal_reading      = sdr->m_data[31];
  m_normal_max           = sdr->m_data[32];
  m_normal_min           = sdr->m_data[33];
  m_sensor_max           = sdr->m_data[34];
  m_sensor_min           = sdr->m_data[35];
  m_upper_non_recoverable_threshold = sdr->m_data[36];
  m_upper_critical_threshold = sdr->m_data[37];
  m_upper_non_critical_threshold = sdr->m_data[38];
  m_lower_non_recoverable_threshold = sdr->m_data[39];
  m_lower_critical_threshold = sdr->m_data[40];
  m_lower_non_critical_threshold = sdr->m_data[41];
  m_positive_going_threshold_hysteresis = sdr->m_data[42];
  m_current_positive_hysteresis = m_positive_going_threshold_hysteresis;
  m_negative_going_threshold_hysteresis = sdr->m_data[43];
  m_current_negative_hysteresis = m_negative_going_threshold_hysteresis;

  double d1, d2;

  m_sensor_factors->ConvertFromRaw( 1, d1, false );
  m_sensor_factors->ConvertFromRaw( 2, d2, false );

  if (d2 < d1)
      m_swap_thresholds = true;
  else
      m_swap_thresholds = false;

  return true;
}


cIpmiSensorFactors *
cIpmiSensorThreshold::CreateSensorFactors( cIpmiMc *mc, cIpmiSdr *sdr )
{
  cIpmiSensorFactors *f= new cIpmiSensorFactors;

  if ( !f->GetDataFromSdr( sdr ) )
     {
       delete f;
       return 0;
     }

  return f;
}


void
cIpmiSensorThreshold::HandleNew( cIpmiDomain *domain )
{
  m_rate_unit_string          = IpmiRateUnitToString( m_rate_unit );
  m_base_unit_string          = IpmiUnitTypeToString( m_base_unit );
  m_modifier_unit_string      = IpmiUnitTypeToString( m_modifier_unit );

  cIpmiSensor::HandleNew( domain );
}


bool
cIpmiSensorThreshold::Cmp( const cIpmiSensor &s2 ) const
{
  if ( cIpmiSensor::Cmp( s2 ) == false )
       return false;

  const cIpmiSensorThreshold *t = dynamic_cast<const cIpmiSensorThreshold *>( &s2 );

  if ( !t )
       return false;

  if ( m_sensor_init_thresholds  != t->m_sensor_init_thresholds )
       return false;

  if ( m_sensor_init_hysteresis  != t->m_sensor_init_hysteresis )
       return false;

  if ( m_hysteresis_support      != t->m_hysteresis_support )
       return false;

  if ( m_threshold_access        != t->m_threshold_access )
       return false;

  if ( m_assertion_event_mask    != t->m_assertion_event_mask )
       return false;
       
  if ( m_deassertion_event_mask  != t->m_deassertion_event_mask )
       return false;

  if ( m_reading_mask            != t->m_reading_mask )
       return false;

  if ( m_threshold_readable      != t->m_threshold_readable )
       return false;

  if ( m_threshold_settable      != t->m_threshold_settable )
       return false;

  if ( m_rate_unit          != t->m_rate_unit )
       return false;

  if ( m_modifier_unit_use  != t->m_modifier_unit_use )
       return false;

  if ( m_percentage         != t->m_percentage )
       return false;

  if ( m_base_unit          != t->m_base_unit )
       return false;

  if ( m_modifier_unit      != t->m_modifier_unit )
       return false;

  bool sf1 = m_sensor_factors ? true : false;
  bool sf2 = t->m_sensor_factors ? true : false;

  if ( sf1 != sf2 )
       return false;

  if ( m_sensor_factors )
       if ( m_sensor_factors->Cmp( *t->m_sensor_factors ) == false )
            return false;

  if ( m_normal_min_specified != t->m_normal_min_specified )
       return false;

  if ( m_normal_max_specified != t->m_normal_max_specified )
       return false;

  if ( m_nominal_reading_specified != t->m_nominal_reading_specified )
       return false;

  if ( m_nominal_reading != t->m_nominal_reading )
       return false;

  if ( m_normal_max != t->m_normal_max )
       return false;

  if ( m_normal_min != t->m_normal_min )
       return false;

  if ( m_sensor_max != t->m_sensor_max )
       return false;

  if ( m_sensor_min != t->m_sensor_min )
       return false;
  if (    m_upper_non_recoverable_threshold
       != t->m_upper_non_recoverable_threshold )
       return false;

  if ( m_upper_critical_threshold != t->m_upper_critical_threshold )
       return false;

  if (    m_upper_non_critical_threshold
       != t->m_upper_non_critical_threshold )
       return false;

  if (    m_lower_non_recoverable_threshold
       != t->m_lower_non_recoverable_threshold )
       return false;

  if ( m_lower_critical_threshold != t->m_lower_critical_threshold )
       return false;

  if ( m_lower_non_critical_threshold
      != t->m_lower_non_critical_threshold )
       return false;

  if (    m_positive_going_threshold_hysteresis
       != t->m_positive_going_threshold_hysteresis )
       return false;

  if (    m_negative_going_threshold_hysteresis
       != t->m_negative_going_threshold_hysteresis )
       return false;

  return true;
}


bool
cIpmiSensorThreshold::IsThresholdReadable( tIpmiThresh event )
{
  return m_threshold_readable & ( 1 << event );
}


bool
cIpmiSensorThreshold::IsThresholdSettable( tIpmiThresh event )
{
  return m_threshold_settable & ( 1 << event );
}

static void
SwapEventState( SaHpiEventStateT &event_state )
{
    switch (event_state)
    {
    case SAHPI_ES_LOWER_MINOR:
        event_state = SAHPI_ES_UPPER_MINOR;
        break;
    case SAHPI_ES_LOWER_MAJOR:
        event_state = SAHPI_ES_UPPER_MAJOR;
        break;
    case SAHPI_ES_LOWER_CRIT:
        event_state = SAHPI_ES_UPPER_CRIT;
        break;
    case SAHPI_ES_UPPER_MINOR:
        event_state = SAHPI_ES_LOWER_MINOR;
        break;
    case SAHPI_ES_UPPER_MAJOR:
        event_state = SAHPI_ES_LOWER_MAJOR;
        break;
    case SAHPI_ES_UPPER_CRIT:
        event_state = SAHPI_ES_LOWER_CRIT;
        break;
    }
}

SaErrorT
cIpmiSensorThreshold::CreateEvent( cIpmiEvent *event, SaHpiEventT &h )
{
  SaErrorT rv = cIpmiSensor::CreateEvent( event, h );

  if ( rv != SA_OK )
       return rv;

  // sensor event
  SaHpiSensorEventT &se = h.EventDataUnion.SensorEvent;

  se.Assertion = (SaHpiBoolT)!(event->m_data[9] & 0x80);

  tIpmiThresh threshold = (tIpmiThresh)((event->m_data[10] >> 1) & 0x07);

  switch( threshold )
     {
       case eIpmiLowerNonCritical:
            se.EventState = SAHPI_ES_LOWER_MINOR;
            h.Severity    = SAHPI_MINOR;
            break;

       case eIpmiLowerCritical:
            se.EventState = SAHPI_ES_LOWER_MAJOR;
            h.Severity    = SAHPI_MAJOR;
            break;

       case eIpmiLowerNonRecoverable:
            se.EventState = SAHPI_ES_LOWER_CRIT;
            h.Severity    = SAHPI_CRITICAL;
            break;

       case eIpmiUpperNonCritical:
            se.EventState = SAHPI_ES_UPPER_MINOR;
            h.Severity    = SAHPI_MINOR;
            break;

       case eIpmiUpperCritical:
            se.EventState = SAHPI_ES_UPPER_MAJOR;
            h.Severity    = SAHPI_MAJOR;
            break;

       case eIpmiUpperNonRecoverable:
            se.EventState = SAHPI_ES_UPPER_CRIT;
            h.Severity = SAHPI_CRITICAL;
            break;

       default:
            stdlog << "Invalid threshold giving !\n";
            se.EventState = SAHPI_ES_UNSPECIFIED;
     }

  if ( SwapThresholds() == true )
  {
      SwapEventState( se.EventState );
  }

  SaHpiSensorOptionalDataT optional_data = 0;

  // byte 2
  tIpmiEventType type = (tIpmiEventType)(event->m_data[10] >> 6);

  if ( type == eIpmiEventData1 )
  {
       ConvertToInterpreted( event->m_data[11],  se.TriggerReading );
       optional_data |= SAHPI_SOD_TRIGGER_READING;
  }
  else if ( type == eIpmiEventData2 )
  {
       se.Oem = (SaHpiUint32T)event->m_data[11]; 
       optional_data |= SAHPI_SOD_OEM;
  }
  else if ( type == eIpmiEventData3 )
  {
       se.SensorSpecific = (SaHpiUint32T)event->m_data[11]; 
       optional_data |= SAHPI_SOD_SENSOR_SPECIFIC;
  }

  // byte 3
  type = (tIpmiEventType)((event->m_data[10] & 0x30) >> 4);

  if ( type == eIpmiEventData1 )
  {
       ConvertToInterpreted( event->m_data[12], se.TriggerThreshold );
       optional_data |= SAHPI_SOD_TRIGGER_THRESHOLD;
  }
  else if ( type == eIpmiEventData2 )
  {
       se.Oem |= (SaHpiUint32T)((event->m_data[12] << 8) & 0xff00);
       optional_data |= SAHPI_SOD_OEM;
  }
  else if ( type == eIpmiEventData3 )
  {
       se.SensorSpecific |= (SaHpiUint32T)((event->m_data[12] << 8) & 0xff00);
       optional_data |= SAHPI_SOD_SENSOR_SPECIFIC;
  }

  se.OptionalDataPresent = optional_data;

  return SA_OK;
}


void 
cIpmiSensorThreshold::Dump( cIpmiLog &dump ) const
{
  cIpmiSensor::Dump( dump );

  dump << "\tthreshold_access " << IpmiThresholdAccessSupportToString( m_threshold_access )
       << ", hysteresis_support " << IpmiHysteresisSupportToString( m_hysteresis_support )
       << " \n";
}


unsigned short
cIpmiSensorThreshold::GetEventMask(unsigned int ipmi_event_mask)
{
  // convert ipmi event mask to hpi event mask
  unsigned short amask = ipmi_event_mask;
  unsigned short mask  = 0;

  for( int i = 0; i < 12; i++ )
       if ( amask & (1 <<i ) )
            mask |= (1 << (i/2));

  return mask;
}


SaErrorT
cIpmiSensorThreshold::ConvertFromInterpreted( const SaHpiSensorReadingT r,
                                              unsigned char &v )
{
    return ( ConvertFromInterpreted( r, v, false ) );
}

SaErrorT
cIpmiSensorThreshold::ConvertFromInterpreted( const SaHpiSensorReadingT r,
                                              unsigned char &v,
                                              bool is_hysteresis)
{
  if ( r.IsSupported == SAHPI_FALSE )
       return SA_OK;

  if ( r.Type != SAHPI_SENSOR_READING_TYPE_FLOAT64 )
       return SA_ERR_HPI_INVALID_DATA;

  unsigned int raw;

  if ( !m_sensor_factors->ConvertToRaw( cIpmiSensorFactors::eRoundNormal, 
                                        (double)r.Value.SensorFloat64,
                                        raw,
                                        is_hysteresis,
                                        SwapThresholds() ) )
       return SA_ERR_HPI_INVALID_DATA;

  v = (unsigned char)raw;

  return SA_OK;
}


void
cIpmiSensorThreshold::ConvertToInterpreted( unsigned int v, SaHpiSensorReadingT &r )
{
    ConvertToInterpreted( v, r, false );
}

void
cIpmiSensorThreshold::ConvertToInterpreted( unsigned int v,
                                            SaHpiSensorReadingT &r,
                                            bool is_hysteresis)
{
  memset( &r, 0, sizeof( SaHpiSensorReadingT ) );

  double d;

  r.IsSupported   = SAHPI_FALSE;
  
  if ( m_sensor_factors->ConvertFromRaw( v, d, is_hysteresis ) )
     {
       r.IsSupported         = SAHPI_TRUE;
       r.Type                = SAHPI_SENSOR_READING_TYPE_FLOAT64;
       r.Value.SensorFloat64 = (SaHpiFloat64T)d;
     }
}

void
static SwapThresholdsMask( SaHpiSensorThdMaskT &threshold_mask )
{
    SaHpiSensorThdMaskT temp_mask;

    temp_mask = threshold_mask;

    threshold_mask = 0;

    if (temp_mask & SAHPI_STM_LOW_MINOR)
        threshold_mask |= SAHPI_STM_UP_MINOR;

    if (temp_mask & SAHPI_STM_LOW_MAJOR)
        threshold_mask |= SAHPI_STM_UP_MAJOR;

    if (temp_mask & SAHPI_STM_LOW_CRIT)
        threshold_mask |= SAHPI_STM_UP_CRIT;

    if (temp_mask & SAHPI_STM_UP_MINOR)
        threshold_mask |= SAHPI_STM_LOW_MINOR;

    if (temp_mask & SAHPI_STM_UP_MAJOR)
        threshold_mask |= SAHPI_STM_LOW_MAJOR;

    if (temp_mask & SAHPI_STM_UP_CRIT)
        threshold_mask |= SAHPI_STM_LOW_CRIT;

    if (temp_mask & SAHPI_STM_UP_HYSTERESIS)
        threshold_mask |= SAHPI_STM_LOW_HYSTERESIS;

    if (temp_mask & SAHPI_STM_LOW_HYSTERESIS)
        threshold_mask |= SAHPI_STM_UP_HYSTERESIS;
}


bool
cIpmiSensorThreshold::CreateRdr( SaHpiRptEntryT &resource,
                                 SaHpiRdrT &rdr )
{
  if ( cIpmiSensor::CreateRdr( resource, rdr ) == false )
       return false;

  SaHpiSensorRecT &rec = rdr.RdrTypeUnion.SensorRec;
  
  // data format
  rec.DataFormat.IsSupported   = SAHPI_TRUE;
  rec.DataFormat.ReadingType   = SAHPI_SENSOR_READING_TYPE_FLOAT64;
  rec.DataFormat.BaseUnits     = (SaHpiSensorUnitsT)BaseUnit();
  rec.DataFormat.ModifierUnits = (SaHpiSensorUnitsT)ModifierUnit();
  rec.DataFormat.ModifierUse   = (SaHpiSensorModUnitUseT)ModifierUnitUse();
  rec.DataFormat.Percentage    = (SaHpiBoolT)Percentage();
  rec.DataFormat.AccuracyFactor = (SaHpiFloat64T)GetFactors()->AccuracyFactor();

  rec.DataFormat.Range.Flags = SAHPI_SRF_MAX | SAHPI_SRF_MIN;

  if ( SwapThresholds() == true )
  {
    ConvertToInterpreted( SensorMax(), rec.DataFormat.Range.Min );
    ConvertToInterpreted( SensorMin(), rec.DataFormat.Range.Max );
  }
  else
  {
    ConvertToInterpreted( SensorMax(), rec.DataFormat.Range.Max );
    ConvertToInterpreted( SensorMin(), rec.DataFormat.Range.Min );
  }

  if ( NominalReadingSpecified() )
     {
       rec.DataFormat.Range.Flags |= SAHPI_SRF_NOMINAL;
       ConvertToInterpreted( NominalReading(), rec.DataFormat.Range.Nominal );
     }

  if ( NormalMaxSpecified() )
     {
       if ( SwapThresholds() == true )
       {
           rec.DataFormat.Range.Flags |= SAHPI_SRF_NORMAL_MIN;
           ConvertToInterpreted( NormalMax(), rec.DataFormat.Range.NormalMin );
       }
       else
       {
           rec.DataFormat.Range.Flags |= SAHPI_SRF_NORMAL_MAX;
           ConvertToInterpreted( NormalMax(), rec.DataFormat.Range.NormalMax );
       }
     }

  if ( NormalMinSpecified() )
     {
       if ( SwapThresholds() == true )
       {
           rec.DataFormat.Range.Flags |= SAHPI_SRF_NORMAL_MAX;
           ConvertToInterpreted( NormalMin(), rec.DataFormat.Range.NormalMax );
       }
       else
       {
           rec.DataFormat.Range.Flags |= SAHPI_SRF_NORMAL_MIN;
           ConvertToInterpreted( NormalMin(), rec.DataFormat.Range.NormalMin );
       }
     }

  // thresholds
  unsigned int        acc = ThresholdAccess();

  if ( acc >= eIpmiThresholdAccessSupportReadable )
     {
       rec.ThresholdDefn.IsAccessible = SAHPI_TRUE;

       SaHpiSensorThdMaskT temp = 0;

       int val = IsThresholdReadable( eIpmiLowerNonCritical );
       if ( val )
           temp |= SAHPI_STM_LOW_MINOR;

       val = IsThresholdReadable( eIpmiLowerCritical );

       if ( val )
           temp |= SAHPI_STM_LOW_MAJOR;

       val = IsThresholdReadable( eIpmiLowerNonRecoverable );
       if ( val )
           temp |= SAHPI_STM_LOW_CRIT;
			
       val = IsThresholdReadable( eIpmiUpperNonCritical );
       if ( val )
           temp |= SAHPI_STM_UP_MINOR;
			
       val = IsThresholdReadable( eIpmiUpperCritical );
       if ( val )
           temp |= SAHPI_STM_UP_MAJOR;
			
       val = IsThresholdReadable( eIpmiUpperNonRecoverable );
       if ( val )
           temp |= SAHPI_STM_UP_CRIT;

       if (    HysteresisSupport() == eIpmiHysteresisSupportReadable 
            || HysteresisSupport() == eIpmiHysteresisSupportSettable ) 
            temp |=   SAHPI_STM_UP_HYSTERESIS
                    | SAHPI_STM_LOW_HYSTERESIS;

       if ( SwapThresholds() == true )
       {
           SwapThresholdsMask( temp );
       }

       rec.ThresholdDefn.ReadThold = temp;
     }

  if ( acc == eIpmiThresholdAccessSupportSettable )
     {
       SaHpiSensorThdMaskT temp = 0;
       int val = IsThresholdSettable( eIpmiLowerNonCritical );
       if ( val )
           temp |= SAHPI_STM_LOW_MINOR;

       val = IsThresholdSettable( eIpmiLowerCritical );

       if ( val )
           temp |= SAHPI_STM_LOW_MAJOR;

       val = IsThresholdSettable( eIpmiLowerNonRecoverable );
       if ( val )
           temp |= SAHPI_STM_LOW_CRIT;
			
       val = IsThresholdSettable( eIpmiUpperNonCritical );
       if ( val )
           temp |= SAHPI_STM_UP_MINOR;
			
       val = IsThresholdSettable( eIpmiUpperCritical );
       if ( val )
           temp |= SAHPI_STM_UP_MAJOR;
			
       val = IsThresholdSettable( eIpmiUpperNonRecoverable );
       if ( val )
           temp |= SAHPI_STM_UP_CRIT;

       if ( HysteresisSupport() == eIpmiHysteresisSupportSettable )
            temp |=   SAHPI_STM_UP_HYSTERESIS
                    | SAHPI_STM_LOW_HYSTERESIS;

       if ( SwapThresholds() == true )
       {
           SwapThresholdsMask( temp );
       }

       rec.ThresholdDefn.WriteThold = temp;
     }

  if ( SwapThresholds() == true )
      {
            SwapEventState(rec.Events);
            SwapEventState(m_current_hpi_assert_mask);
            SwapEventState(m_current_hpi_deassert_mask);
            SwapEventState(m_hpi_assert_mask);
            SwapEventState(m_hpi_deassert_mask);
      }

  rec.ThresholdDefn.Nonlinear = GetFactors()->IsNonLinear();

  return true;
}


SaErrorT
cIpmiSensorThreshold::GetSensorReading( SaHpiSensorReadingT &data,
                                        SaHpiEventStateT &state )
{
  if ( m_enabled == SAHPI_FALSE )
      return SA_ERR_HPI_INVALID_REQUEST;

  cIpmiMsg rsp;
  SaErrorT rv = GetSensorData( rsp );

  if ( rv != SA_OK )
       return rv;

  if ( &data != NULL )
      ConvertToInterpreted( rsp.m_data[1], data );

  if ( &state != NULL )
  {
      state = rsp.m_data[3] & 0x3f;

      if ( SwapThresholds() == true )
      {
        SwapEventState( state );
      }
  }

  return SA_OK;
}


SaErrorT
cIpmiSensorThreshold::GetDefaultThresholds( SaHpiSensorThresholdsT &thres )
{
  if ( IsThresholdReadable( eIpmiLowerNonRecoverable ) )
       ConvertToInterpreted( m_lower_non_recoverable_threshold, thres.LowCritical );

  if ( IsThresholdReadable( eIpmiLowerCritical ) )
       ConvertToInterpreted( m_lower_critical_threshold, thres.LowMajor );

  if ( IsThresholdReadable( eIpmiLowerNonCritical ) )
       ConvertToInterpreted( m_lower_non_critical_threshold, thres.LowMinor );

  if ( IsThresholdReadable( eIpmiUpperNonRecoverable ) )
       ConvertToInterpreted( m_upper_non_recoverable_threshold, thres.UpCritical );

  if ( IsThresholdReadable( eIpmiUpperCritical ) )
       ConvertToInterpreted( m_upper_critical_threshold, thres.UpMajor );
  
  if ( IsThresholdReadable( eIpmiUpperNonCritical ) )
       ConvertToInterpreted( m_upper_non_critical_threshold, thres.UpMinor );

  return SA_OK;
}


SaErrorT
cIpmiSensorThreshold::GetThresholds( SaHpiSensorThresholdsT &thres )
{
  cIpmiResource *res = Resource();

  stdlog << "read thresholds for sensor " << EntityPath() << " num " 
         << m_num << " " << IdString() << ".\n";

  if ( m_threshold_access == eIpmiThresholdAccessSupportFixed )
       // Thresholds are fixed, pull them from the SDR.
       return GetDefaultThresholds( thres );

  cIpmiMsg msg( eIpmiNetfnSensorEvent, eIpmiCmdGetSensorThreshold );
  cIpmiMsg rsp;

  msg.m_data_len = 1;
  msg.m_data[0]  = m_num;

  SaErrorT rv = res->SendCommandReadLock( this, msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "error getting thresholds: " << rv << " !\n";

       return rv;
     }

  if ( rsp.m_data[0] )
     {
       stdlog << "IPMI error getting thresholds: " << rsp.m_data[0] << " !\n";

       return SA_ERR_HPI_INVALID_DATA;
    }

  if ( rsp.m_data[1] & (1 << eIpmiLowerNonRecoverable) )
       ConvertToInterpreted( rsp.m_data[4], thres.LowCritical );

  if ( rsp.m_data[1] & (1 << eIpmiLowerCritical ) )
       ConvertToInterpreted( rsp.m_data[3], thres.LowMajor );

  if ( rsp.m_data[1] & (1 << eIpmiLowerNonCritical ) )
       ConvertToInterpreted( rsp.m_data[2], thres.LowMinor );

  if ( rsp.m_data[1] & (1 << eIpmiUpperNonRecoverable ) )
       ConvertToInterpreted( rsp.m_data[7], thres.UpCritical );

  if ( rsp.m_data[1] & (1 << eIpmiUpperCritical ) )
       ConvertToInterpreted( rsp.m_data[6], thres.UpMajor );

  if ( rsp.m_data[1] & (1 << eIpmiUpperNonCritical ) )
       ConvertToInterpreted( rsp.m_data[5], thres.UpMinor );

  return SA_OK;
}


SaErrorT
cIpmiSensorThreshold::GetHysteresis( SaHpiSensorThresholdsT &thres )
{
  cIpmiResource *res = Resource();

  stdlog << "read hysteresis for sensor " << EntityPath() << " num " << m_num 
         << " " << IdString() << ".\n";

  if (    m_hysteresis_support != eIpmiHysteresisSupportReadable
       && m_hysteresis_support != eIpmiHysteresisSupportSettable)
       return SA_OK;

  cIpmiMsg msg( eIpmiNetfnSensorEvent, eIpmiCmdGetSensorHysteresis );
  cIpmiMsg rsp;

  msg.m_data_len = 2;
  msg.m_data[0]  = m_num;
  msg.m_data[1]  = 0xff;

  SaErrorT rv = res->SendCommandReadLock( this, msg, rsp, m_lun );

  if ( rv )
     {
       stdlog << "Error sending hysteresis get command: " << rv << " !\n";

       return rv;
     }

  if ( rsp.m_data[0] || rsp.m_data_len < 3 )
     {
       stdlog << "IPMI error getting hysteresis: " << rsp.m_data[0] << "!\n";

       return SA_ERR_HPI_INVALID_CMD;
    }

  m_current_positive_hysteresis = rsp.m_data[1];
  m_current_negative_hysteresis = rsp.m_data[2];

  ConvertToInterpreted( rsp.m_data[1], thres.PosThdHysteresis, true );
  ConvertToInterpreted( rsp.m_data[2], thres.NegThdHysteresis, true );

  return SA_OK;
}

void static
SwapThresholdsReading( SaHpiSensorThresholdsT &thres )
{
  SaHpiSensorThresholdsT tmp_tres;

  memcpy( &tmp_tres, &thres, sizeof( SaHpiSensorThresholdsT ));

  memcpy( &thres.LowCritical, &tmp_tres.UpCritical, sizeof( SaHpiSensorReadingT ) );
  memcpy( &thres.LowMajor,    &tmp_tres.UpMajor,    sizeof( SaHpiSensorReadingT ) );
  memcpy( &thres.LowMinor,    &tmp_tres.UpMinor,    sizeof( SaHpiSensorReadingT ) );

  memcpy( &thres.UpCritical, &tmp_tres.LowCritical, sizeof( SaHpiSensorReadingT ) );
  memcpy( &thres.UpMajor,    &tmp_tres.LowMajor,    sizeof( SaHpiSensorReadingT ) );
  memcpy( &thres.UpMinor,    &tmp_tres.LowMinor,    sizeof( SaHpiSensorReadingT ) );

  memcpy( &thres.PosThdHysteresis, &tmp_tres.NegThdHysteresis, sizeof( SaHpiSensorReadingT ) );
  memcpy( &thres.NegThdHysteresis, &tmp_tres.PosThdHysteresis, sizeof( SaHpiSensorReadingT ) );
}

SaErrorT
cIpmiSensorThreshold::GetThresholdsAndHysteresis( SaHpiSensorThresholdsT &thres )
{
  SaErrorT rv;

  memset( &thres, 0, sizeof( SaHpiSensorThresholdsT ) );

  bool found = false;

  if ( ThresholdAccess() == eIpmiThresholdAccessSupportNone )
       stdlog << "sensor doesn't support threshold read !\n";
  else
     {
       rv = GetThresholds( thres );

       if ( rv != SA_OK )
            return rv;

       found = true;
     }

  if (    HysteresisSupport() == eIpmiHysteresisSupportReadable
       || HysteresisSupport() == eIpmiHysteresisSupportSettable )
     {
       rv = GetHysteresis( thres );

       if ( rv != SA_OK )
            return rv;

       found = true;
     }
  else
       stdlog << "sensor doesn't support hysteresis read !\n";

  if ( !found )
       return SA_ERR_HPI_INVALID_CMD;

  if ( SwapThresholds() == true )
  {
      SwapThresholdsReading( thres );
  }

  return SA_OK;
}


SaErrorT
cIpmiSensorThreshold::ConvertThreshold( const SaHpiSensorReadingT &r, 
                                        tIpmiThresh event,
                                        unsigned char &data,
                                        unsigned char &mask )
{
  // convert the interpreted data
  SaErrorT rv = ConvertFromInterpreted( r, data );

  if ( rv != SA_OK )
       return rv;

  if ( r.IsSupported == SAHPI_TRUE )
       mask |= (1 << event);

  return SA_OK;
}


SaErrorT
cIpmiSensorThreshold::SetThresholds( const SaHpiSensorThresholdsT &thres )
{
  stdlog << "write thresholds for sensor " << EntityPath() << " num " 
         << m_num << " " << IdString() << ".\n";

  cIpmiMsg msg( eIpmiNetfnSensorEvent, eIpmiCmdSetSensorThreshold );
  memset( msg.m_data, 0, dIpmiMaxMsgLength );

  msg.m_data_len = 8;
  msg.m_data[0]  = m_num;
  msg.m_data[1]  = 0;

  SaErrorT rv;

  rv = ConvertThreshold( thres.LowMinor, eIpmiLowerNonCritical,
                         msg.m_data[2], msg.m_data[1] );

  if ( rv != SA_OK )
       return rv;

  rv = ConvertThreshold( thres.LowMajor, eIpmiLowerCritical,
                         msg.m_data[3], msg.m_data[1] );

  if ( rv != SA_OK )
       return rv;

  rv = ConvertThreshold( thres.LowCritical, eIpmiLowerNonRecoverable,
                         msg.m_data[4], msg.m_data[1] );

  if ( rv != SA_OK )
       return rv;

  rv = ConvertThreshold( thres.UpMinor, eIpmiUpperNonCritical,
                         msg.m_data[5], msg.m_data[1] );

  if ( rv != SA_OK )
       return rv;

  rv = ConvertThreshold( thres.UpMajor, eIpmiUpperCritical,
                         msg.m_data[6], msg.m_data[1] );

  if ( rv != SA_OK )
       return rv;

  rv = ConvertThreshold( thres.UpCritical, eIpmiUpperNonRecoverable,
                         msg.m_data[7], msg.m_data[1] );

  if ( rv != SA_OK )
       return rv;

  // nothing to do
  if ( msg.m_data[1] == 0 )
       return SA_OK;

  // settable ?
  if ( m_threshold_access != eIpmiThresholdAccessSupportSettable )
       return SA_ERR_HPI_INVALID_CMD;

  if ( (m_threshold_settable | msg.m_data[1]) != m_threshold_settable )
       return SA_ERR_HPI_INVALID_CMD;

  // set thresholds
  cIpmiMsg rsp;

  rv = Resource()->SendCommandReadLock( this, msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "Error sending thresholds set command: " << rv << " !\n";
       return rv;
     }

  if ( rsp.m_data[0] )
     {
       stdlog << "IPMI error setting thresholds: " << rsp.m_data[0] << " !\n";
       return SA_ERR_HPI_INVALID_CMD;
    }

  return SA_OK;
}


SaErrorT
cIpmiSensorThreshold::SetHysteresis( const SaHpiSensorThresholdsT &thres )
{
  SaErrorT rv;

  // nothing to do
  if (    (thres.PosThdHysteresis.IsSupported == SAHPI_FALSE)
       && (thres.NegThdHysteresis.IsSupported == SAHPI_FALSE) )
       return SA_OK;

  if ( m_hysteresis_support != eIpmiHysteresisSupportSettable )
       return SA_ERR_HPI_INVALID_CMD;

  cIpmiMsg  msg( eIpmiNetfnSensorEvent, eIpmiCmdSetSensorHysteresis );
  cIpmiMsg  rsp;

  msg.m_data_len = 4;
  msg.m_data[0]  = m_num;
  msg.m_data[1]  = 0xff;

  if (thres.PosThdHysteresis.IsSupported == SAHPI_FALSE)
  {
      msg.m_data[2] = m_current_positive_hysteresis;
  }
  else
  {
      rv = ConvertFromInterpreted( thres.PosThdHysteresis, msg.m_data[2], true );

      if ( rv != SA_OK )
          return rv;

      m_current_positive_hysteresis = msg.m_data[2];
  }

  if (thres.NegThdHysteresis.IsSupported == SAHPI_FALSE)
  {
      msg.m_data[3] = m_current_negative_hysteresis;
  }
  else
  {
      rv = ConvertFromInterpreted( thres.NegThdHysteresis, msg.m_data[3], true );

      if ( rv != SA_OK )
          return rv;

      m_current_negative_hysteresis = msg.m_data[3];
  }

  rv = Resource()->SendCommandReadLock( this, msg, rsp, m_lun );

  if ( rv != SA_OK )
     {
       stdlog << "Error sending hysteresis set command: " << rv << " !\n";

       return rv;
     }

  if ( rsp.m_data[0] )
     {
       stdlog << "IPMI error setting hysteresis: " << rsp.m_data[0] << " !\n";
       return SA_ERR_HPI_INVALID_CMD;
     }

  return SA_OK;
}


SaErrorT
cIpmiSensorThreshold::SetThresholdsAndHysteresis( const SaHpiSensorThresholdsT &thres )
{
  SaErrorT rv;
  SaHpiSensorThresholdsT tmp_tres;

  memcpy( &tmp_tres, &thres, sizeof( SaHpiSensorThresholdsT ) );

  if ( SwapThresholds() == true )
  {
      SwapThresholdsReading( tmp_tres );
  }

  if ( ThresholdAccess() == eIpmiThresholdAccessSupportSettable )
     {

       rv = SetThresholds( tmp_tres );

       if ( rv != SA_OK )
            return rv;
     }
  else
       stdlog << "sensor doesn't support threshold set !\n";

  if ( HysteresisSupport() == eIpmiHysteresisSupportSettable )
     { 

       rv = SetHysteresis( tmp_tres );

       if ( rv != SA_OK )
            return rv;
     }
  else
       stdlog << "sensor doesn't support hysteresis set !\n";

  return SA_OK;
}


SaErrorT
cIpmiSensorThreshold::GetEventMasksHw( SaHpiEventStateT &AssertEventMask,
                                       SaHpiEventStateT &DeassertEventMask
                                     )
{
  AssertEventMask = 0;
  DeassertEventMask = 0;

  cIpmiMsg rsp;
  SaErrorT rv = cIpmiSensor::GetEventMasksHw( rsp );

  if ( rv != SA_OK )
       return rv;

  unsigned int amask = IpmiGetUint16( rsp.m_data + 2 );
  unsigned int dmask = IpmiGetUint16( rsp.m_data + 4 );

  for( int i = 0; i < 6; i++ )
     {
       unsigned int b1 = 1 << (2*i);
       unsigned int b2 = 1 << (2*i + 1);

       if ( (amask & b1) || (amask & b2) )
            AssertEventMask |= (1 << i);

       if ( (dmask & b1) || (dmask & b2) )
            DeassertEventMask |= (1 << i);
     }

  if ( SwapThresholds() == true )
  {
      SwapEventState( AssertEventMask );
      SwapEventState( DeassertEventMask );
  }

  return SA_OK;
}


SaErrorT
cIpmiSensorThreshold::SetEventMasksHw( const SaHpiEventStateT &AssertEventMask,
                                       const SaHpiEventStateT &DeassertEventMask
                                     )
{
  // create de/assertion event mask
  unsigned int amask = 0;
  unsigned int dmask = 0;

  SaHpiEventStateT assert_mask = AssertEventMask;
  SaHpiEventStateT deassert_mask = DeassertEventMask;

  if ( SwapThresholds() == true )
  {
      SwapEventState( assert_mask );
      SwapEventState( deassert_mask );
  }

  for( int i = 0; i < 6; i++ )
     {
       unsigned int b1 = 1 << (2*i);
       unsigned int b2 = 1 << (2*i + 1);
       unsigned int b  = b1 | b2; // this is 3 << (2*i)

       if ( assert_mask & ( 1 << i ) )
          {
            if ( (m_assertion_event_mask & b) == 0 )
               {
                 // this event is not allowed
                 stdlog << "SetEventEnables: assertion event "
                        << IpmiThresToString( (tIpmiThresh)i )
                        << " not allowed !\n";

                 return SA_ERR_HPI_INVALID_DATA;
               }

            amask |= (m_assertion_event_mask & b);
          }

       if ( deassert_mask & ( 1 << i ) )
          {
            if ( (m_deassertion_event_mask & b) == 0 )
               {
                 // this event is not allowed
                 stdlog << "SetEventEnables: deassertion event " 
                        << IpmiThresToString( (tIpmiThresh)i )
                        << " not allowed !\n";

                 return SA_ERR_HPI_INVALID_DATA; 
               }

            dmask |= (m_deassertion_event_mask & b);
          }
     }

  cIpmiMsg msg;
  SaErrorT rv = SA_OK;

  if (( amask != 0 )
      || ( dmask != 0 ))
  {
    IpmiSetUint16( msg.m_data + 2, amask );
    IpmiSetUint16( msg.m_data + 4, dmask );

    rv = cIpmiSensor::SetEventMasksHw( msg, true );
  }

  if ( rv != SA_OK )
      return rv;

  amask = ( amask ^ m_assertion_event_mask ) & m_assertion_event_mask;
  dmask = ( dmask ^ m_deassertion_event_mask ) & m_deassertion_event_mask;

  if (( amask != 0 )
      || ( dmask != 0 ))
  {
    IpmiSetUint16( msg.m_data + 2, amask );
    IpmiSetUint16( msg.m_data + 4, dmask );

    rv = cIpmiSensor::SetEventMasksHw( msg, false );
  }

  return rv;
}
