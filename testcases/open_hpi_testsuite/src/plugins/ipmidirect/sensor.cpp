/*
 *
 * Copyright (c) 2003,2004 by FORCE Computers.
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
 */


#include "ipmi.h"
#include "ipmi_sensor.h"
#include "ipmi_utils.h"


enum tOhEventType
{
  eEventData0 = 0,
  eEventData1,
  eEventData2,
  eEventData3
};


// IPMI does not define the decrete sensor event state
enum tIpmiDiscrete
{
  eIpmiTransIdle = 0,
  eIpmiTransActive,
  eIpmiTransBusy
};


SaErrorT
cIpmi::IfGetSensorData( cIpmiSensor *sensor, SaHpiSensorReadingT &data )
{
  tIpmiStates states;
  int         rv;

  if ( sensor->Ignore() )
     {
       dbg("sensor is ignored");
       return SA_ERR_HPI_NOT_PRESENT;
     }

  if ( sensor->EventReadingType() == eIpmiEventReadingTypeThreshold )
     {
       tIpmiValuePresent value_present;
       unsigned int      raw_val;
       double            val;

       rv = sensor->ReadingGet( value_present,
                                raw_val, val,
                                states );

       if ( rv )
          {
            IpmiLog( "Unable to get sensor reading: %s !\n",
                     strerror( rv ) );
            return SA_ERR_HPI_INVALID_CMD;
          }

       if ( value_present == eIpmiBothValuesPresent )
          {
            data.ValuesPresent    = SAHPI_SRF_RAW | SAHPI_SRF_INTERPRETED;
            data.Raw              = (SaHpiUint32T)raw_val;
            data.Interpreted.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
            data.Interpreted.Value.SensorFloat32 = (SaHpiFloat32T)val;
          }
       else if ( value_present == eIpmiRawValuePresent )
          {
            data.ValuesPresent = SAHPI_SRF_RAW;
            data.Raw           = (SaHpiUint32T)raw_val;
          }
       else
            data.ValuesPresent = 0;

       return 0;
     }

  // discrete sensor
  rv = sensor->StatesGet( states );

  if ( rv )
     {
       IpmiLog( "Unable to get sensor reading: %s !\n",
                strerror( rv ) );
       return SA_ERR_HPI_INVALID_CMD;
     }

  data.ValuesPresent = SAHPI_SRF_EVENT_STATE;

  if ( states.m_event_messages_enabled )
       data.EventStatus.SensorStatus |= SAHPI_SENSTAT_EVENTS_ENABLED;

  if ( states.m_sensor_scanning_enabled )
       data.EventStatus.SensorStatus |= SAHPI_SENSTAT_EVENTS_ENABLED;

  if ( states.m_initial_update_in_progress )
       data.EventStatus.SensorStatus |= SAHPI_SENSTAT_BUSY;

  data.EventStatus.EventStatus = states.m_states;

  return 0;
}


static void
ThresGet( cIpmiSensor           *sensor,
          const tIpmiThresholds &th,
          unsigned int           event,
          SaHpiSensorReadingT   &thres )
{
  int val;

  val = sensor->ThresholdReadable( (tIpmiThresh)event );

  if ( val )
     {
       thres.ValuesPresent    = SAHPI_SRF_INTERPRETED;
       thres.Interpreted.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
       thres.Interpreted.Value.SensorFloat32 = (SaHpiFloat32T)
         th.m_vals[event].m_val;
     }
}


static int
GetThresholds( cIpmiSensor *sensor,
               SaHpiSensorThresholdsT &thres )
{
  tIpmiThresholds th;
  int rv;

  rv = sensor->ThresholdsGet( th );

  if ( rv )
       return rv;

  ThresGet( sensor, th, eIpmiLowerNonCritical,    thres.LowMinor );
  ThresGet( sensor, th, eIpmiLowerCritical,       thres.LowMajor );
  ThresGet( sensor, th, eIpmiLowerNonRecoverable, thres.LowCritical );
  ThresGet( sensor, th, eIpmiUpperNonCritical,    thres.UpMinor );
  ThresGet( sensor, th, eIpmiUpperCritical,       thres.UpMajor );
  ThresGet( sensor, th, eIpmiUpperNonRecoverable, thres.UpCritical );

  return 0;
}


static int
GetHysteresis( cIpmiSensor *sensor,
               SaHpiSensorThresholdsT &thres )
{
  unsigned int	positive_hysteresis;
  unsigned int 	negative_hysteresis;
  int rv;
  double tmp;

  rv = sensor->GetHysteresis( positive_hysteresis,
                              negative_hysteresis );

  if ( rv )
       return rv;

  // convert to interpreted value
  rv = sensor->ConvertFromRaw( positive_hysteresis, tmp );

  if ( rv < 0 )
     {
       IpmiLog( "Invalid raw value !\n");
       return -1;
     }

  thres.PosThdHysteresis.ValuesPresent       = SAHPI_SRF_INTERPRETED;	
  thres.PosThdHysteresis.Interpreted.Type    = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
  thres.PosThdHysteresis.Interpreted.Value.SensorFloat32 = tmp;

  rv = sensor->ConvertFromRaw( negative_hysteresis, tmp );

  if ( rv < 0 )
     {
       IpmiLog( "Invalid raw value !\n");
       return -1;
     }

  thres.NegThdHysteresis.ValuesPresent       = SAHPI_SRF_INTERPRETED;
  thres.NegThdHysteresis.Interpreted.Type    = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
  thres.NegThdHysteresis.Interpreted.Value.SensorFloat32 = tmp;

  return 0;
}


SaErrorT
cIpmi::IfGetSensorThresholds( cIpmiSensor *sensor, SaHpiSensorThresholdsT &thres )
{
  int rv;

  memset( &thres, 0, sizeof( SaHpiSensorThresholdsT ) );

  if ( sensor->Ignore() )
     {
       IpmiLog( "sensor is ignored !\n");
       return SA_ERR_HPI_NOT_PRESENT;
     }

  if ( sensor->EventReadingType() != eIpmiEventReadingTypeThreshold )
     {
       IpmiLog( "Not threshold sensor!" );

       return SA_ERR_HPI_INVALID_REQUEST;
     }

  if ( sensor->ThresholdAccess() == eIpmiThresholdAccessSupportNone )
       IpmiLog( "sensor doesn't support threshold read !\n" );
  else
     {
       rv = GetThresholds( sensor, thres );

       if ( rv < 0 )
            return SA_ERR_HPI_UNKNOWN;
     }

  if (    sensor->HysteresisSupport() == eIpmiHysteresisSupportReadable
       || sensor->HysteresisSupport() == eIpmiHysteresisSupportSettable )
     {
       rv = GetHysteresis( sensor, thres );

       if ( rv < 0 )
            return SA_ERR_HPI_UNKNOWN;
     }
  else
       IpmiLog( "sensor doesn't support hysteresis read !\n");

  return 0;
}


static int
ThresCpy( cIpmiSensor              *sensor, 
          const SaHpiSensorReadingT reading,
          unsigned int              event,
          tIpmiThresholds          &info ) 
{
  double tmp;
  int    val;
  int    rv;

  val = sensor->ThresholdSettable( (tIpmiThresh)event );

  if ( !val )
       return 0;

  if ( reading.ValuesPresent & SAHPI_SRF_INTERPRETED )
     {
       if ( reading.Interpreted.Type ==
            SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32 )
          {
            info.m_vals[event].m_status = 1;
            info.m_vals[event].m_val = reading.Interpreted.Value.SensorFloat32;
          }
       else
          {
            IpmiLog( "Invalid input thresholds !\n");
            return -1;
          }
     }
  else if ( reading.ValuesPresent & SAHPI_SRF_RAW )
     {
       rv = sensor->ConvertFromRaw( reading.Raw, tmp );

       if ( rv < 0 )
          {
            IpmiLog( "Invalid raw value !\n");
            return -1;
          }

       info.m_vals[event].m_status = 1;
       info.m_vals[event].m_val = tmp;
     }

  return 0;
}


static int
SetThresholds( cIpmiSensor *sensor, 
               const SaHpiSensorThresholdsT &thres )
{
  tIpmiThresholds info;
  int             rv;	

  memset( &info, 0, sizeof(tIpmiThresholds) );

  rv = ThresCpy( sensor, thres.LowMinor, eIpmiLowerNonCritical, info );
  if ( rv < 0 )
       return -1;

  rv = ThresCpy( sensor, thres.LowMajor, eIpmiLowerCritical, info );
  if ( rv < 0 )
       return -1;

  rv = ThresCpy( sensor, thres.LowCritical, eIpmiLowerNonRecoverable,
                 info );
  if ( rv < 0 )
       return -1;

  rv = ThresCpy( sensor, thres.UpMinor, eIpmiUpperNonCritical, info );
  if ( rv < 0 )
       return -1;

  rv = ThresCpy( sensor, thres.UpMajor, eIpmiUpperCritical, info );
  if ( rv < 0 )
       return -1;

  rv = ThresCpy( sensor, thres.UpCritical, eIpmiUpperNonRecoverable,
                 info );
  if ( rv < 0 )
       return -1;

  rv = sensor->ThresholdsSet( info );

  if ( rv)
     {
       IpmiLog( "Unable to set sensor thresholds: 0x%x\n", rv );
       return -1;
     }

  return 0;
}


static int
SetHysteresis( cIpmiSensor *sensor,
               const SaHpiSensorThresholdsT &thres )
{
  int                 rv;	
  double              tmp;
  unsigned int        pos, neg;
  SaHpiSensorReadingT pos_reading = thres.PosThdHysteresis;
  SaHpiSensorReadingT neg_reading = thres.NegThdHysteresis;	

  bool pos_present = true;

  if ( pos_reading.ValuesPresent & SAHPI_SRF_RAW )
       pos = pos_reading.Raw;
  else if ( pos_reading.ValuesPresent & SAHPI_SRF_INTERPRETED )
     {
       if (    pos_reading.Interpreted.Type
            == SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32 )
          {
            tmp = pos_reading.Interpreted.Value.SensorFloat32;
            sensor->ConvertToRaw( cIpmiSensor::eRoundNormal, 
                                  tmp, pos );
          }
       else
          {
            IpmiLog( "Invalid input thresholds !\n");
            return -1;
          }
     }
  else
       pos_present = false;

  bool neg_present = true;

  if ( neg_reading.ValuesPresent & SAHPI_SRF_RAW )
       neg = neg_reading.Raw;
  else if ( neg_reading.ValuesPresent & SAHPI_SRF_INTERPRETED )
     {
       if (    neg_reading.Interpreted.Type
            == SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32 )
          {
            tmp = neg_reading.Interpreted.Value.SensorFloat32;
            sensor->ConvertToRaw( cIpmiSensor::eRoundNormal,
                                  tmp, neg );
          }
       else
          {
            IpmiLog( "Invalid input thresholds !\n");
            return -1;
          }
     }
  else 
       neg_present = false;

  // if both values not present => do nothing
  if ( pos_present == false || neg_present == false )
       return 0;

  // TODO: if only one value is present, we need to read
  // hysteresis first.

  rv = sensor->SetHysteresis( pos, neg );

  if ( rv )
     {
       IpmiLog( "Unable to set sensor thresholds: 0x%x\n", rv );
       return -1;
     }

  return 0;
}


SaErrorT
cIpmi::IfSetSensorThresholds( cIpmiSensor *sensor,
                              const SaHpiSensorThresholdsT &thres )
{
  int rv;	

  if ( sensor->Ignore() )
     {
       IpmiLog( "Sensor is ignored.\n");
       return -1;
     }

  if ( sensor->EventReadingType() != eIpmiEventReadingTypeThreshold )
     {
       IpmiLog( "Not threshold sensor!\n");
       return -1;
     }
  
  if ( sensor->ThresholdAccess() == eIpmiThresholdAccessSupportSettable )
     {
       rv = SetThresholds( sensor, thres );

       if ( rv < 0 )
            return -1;
     }
  else
       IpmiLog( "sensor doesn't support threshold set !\n" );

  if ( sensor->HysteresisSupport() == eIpmiHysteresisSupportSettable )
     { 
       rv = SetHysteresis( sensor, thres );

       if ( rv < 0 )
            return -1;
     }
  else
       IpmiLog( "sensor doesn't support hysteresis set !\n");

  return 0;
}


SaErrorT
cIpmi::IfGetSensorEventEnables( cIpmiSensor *sensor,
                                SaHpiSensorEvtEnablesT &enables )
{
  cIpmiEventState state;
  bool rv;

  if ( sensor->Ignore() )
     {
       IpmiLog( "sensor is ignored !\n");
       return -1;
     }	

  if ( sensor->EventSupport() == eIpmiEventSupportNone )
     {
       IpmiLog("Sensor do not support event !\n");
       return -1;
     }

  memset( &enables, 0, sizeof( SaHpiSensorEvtEnablesT ) );

  rv = sensor->EventsEnableGet( state );

  if ( rv )
     {
       IpmiLog( "Unable to sensor event enables: 0x%x\n", rv );
       return -1;
     }

  rv = state.GetEventsEnabled();
  if ( rv )
       enables.SensorStatus |= SAHPI_SENSTAT_EVENTS_ENABLED;

  rv = state.GetScanningEnabled();
  if ( rv )
       enables.SensorStatus |= SAHPI_SENSTAT_SCAN_ENABLED;

  rv = state.GetBusy();
  if ( rv )
       enables.SensorStatus |= SAHPI_SENSTAT_BUSY;

  enables.AssertEvents = (SaHpiEventStateT)
    state.m_assertion_events;
  enables.DeassertEvents = (SaHpiEventStateT)
    state.m_deassertion_events;

  return 0;
}


SaErrorT
cIpmi::IfSetSensorEventEnables( cIpmiSensor *sensor,
                                const SaHpiSensorEvtEnablesT &enables )
{
  int             rv;
  cIpmiEventState info;
  int i;

  if ( sensor->Ignore() )
     {
       IpmiLog( "sensor is ignored !\n");
       return -1;
     }	

  if ( sensor->EventSupport() == eIpmiEventSupportNone )
     {
       IpmiLog("Sensor do not support event !\n");
       return -1;
     }
 
  info.m_status = enables.SensorStatus;

  if ( enables.AssertEvents == 0xffff )
     {
       /* enable all assertion events */
       info.m_assertion_events = 0;

       for( i = 0; i < 12; i++ )
          {
            int val = 0;

            if ( sensor->EventReadingType() == eIpmiEventReadingTypeThreshold )
                 sensor->ThresholdAssertionEventSupported( (tIpmiThresh)0,
                                                           (tIpmiEventValueDir)i, val );
            else
                 sensor->DiscreteAssertionEventSupported( i, val );

            if ( val )
                 info.m_assertion_events |= (1 << i);
          }
     }
  else
       info.m_assertion_events = enables.AssertEvents;

  if ( enables.DeassertEvents == 0xffff )
     {
       // enable all deassertion events
       info.m_deassertion_events = 0;

       for( i = 0; i < 12; i++ )
          {
            int val = 0;
            
            if ( sensor->EventReadingType() == eIpmiEventReadingTypeThreshold )
                 sensor->ThresholdDeassertionEventSupported( (tIpmiThresh)0,
                                                             (tIpmiEventValueDir)i, val );
            else
                 sensor->DiscreteDeassertionEventSupported( i, val );

            if ( val )
                 info.m_deassertion_events |= (1 << i);
          }
     }
  else
       info.m_deassertion_events = enables.DeassertEvents;

  rv = sensor->EventsEnableSet( info );

  if ( rv )
     {
       IpmiLog( "Unable to sensor event enables: 0x%x !\n", rv );
       return -1;
     }

  return 0;
}


static void
AddSensorEventThresholds( cIpmiSensor	*sensor,
                          SaHpiSensorRecT &rec)
{
  int 			val;
  SaHpiSensorThdMaskT	temp;
  unsigned int		access;

  if ( rec.Category != SAHPI_EC_THRESHOLD )
     {
       rec.ThresholdDefn.IsThreshold = SAHPI_FALSE;
       return;
     }

  access = sensor->ThresholdAccess();

  if ( access == eIpmiThresholdAccessSupportNone )
     {
       rec.ThresholdDefn.IsThreshold = SAHPI_FALSE;
       return;
     }

  if ( access >= eIpmiThresholdAccessSupportReadable )
     {
       rec.ThresholdDefn.IsThreshold = SAHPI_TRUE;
       rec.ThresholdDefn.TholdCapabilities = SAHPI_SRF_RAW | SAHPI_SRF_INTERPRETED;

       temp = 0;
       val = sensor->ThresholdReadable( eIpmiLowerNonCritical );
       if ( val )
            temp |= SAHPI_STM_LOW_MINOR;

       val = sensor->ThresholdReadable( eIpmiLowerCritical );

       if ( val )
            temp |= SAHPI_STM_LOW_MAJOR;

       val = sensor->ThresholdReadable( eIpmiLowerNonRecoverable );
       if ( val )
            temp |= SAHPI_STM_LOW_CRIT;
			
       val = sensor->ThresholdReadable( eIpmiUpperNonCritical );
       if ( val )
            temp |= SAHPI_STM_UP_MINOR;
			
       val = sensor->ThresholdReadable( eIpmiUpperCritical );
       if ( val )
            temp |= SAHPI_STM_UP_MAJOR;
			
       val = sensor->ThresholdReadable( eIpmiUpperNonRecoverable );
       if ( val )
            temp |= SAHPI_STM_UP_CRIT;

       if (    sensor->HysteresisSupport() == eIpmiHysteresisSupportReadable 
            || sensor->HysteresisSupport() == eIpmiHysteresisSupportSettable ) 
            temp |= SAHPI_STM_UP_HYSTERESIS |
              SAHPI_STM_LOW_HYSTERESIS;

       rec.ThresholdDefn.ReadThold = temp;
     }

  if ( access == eIpmiThresholdAccessSupportSettable )
     {
       temp = 0;
       val = sensor->ThresholdSettable( eIpmiLowerNonCritical );

       if ( val )
            temp |= SAHPI_STM_LOW_MINOR;

       val = sensor->ThresholdSettable( eIpmiLowerCritical );
       if ( val )
            temp |= SAHPI_STM_LOW_MAJOR;

       val = sensor->ThresholdSettable( eIpmiLowerNonRecoverable );
       if ( val )
            temp |= SAHPI_STM_LOW_CRIT;

       val = sensor->ThresholdSettable( eIpmiUpperNonCritical );
       if ( val )
            temp |= SAHPI_STM_UP_MINOR;

       val = sensor->ThresholdSettable( eIpmiUpperCritical );
       if ( val )
            temp |= SAHPI_STM_UP_MAJOR;

       val = sensor->ThresholdSettable( eIpmiUpperNonRecoverable );
       if ( val )
            temp |= SAHPI_STM_UP_CRIT;

       if ( sensor->HysteresisSupport() == eIpmiHysteresisSupportSettable ) 
            temp |= SAHPI_STM_UP_HYSTERESIS |
              SAHPI_STM_LOW_HYSTERESIS;

       rec.ThresholdDefn.WriteThold = temp;
     }

  temp = 0;

  if ( sensor->HysteresisSupport() == eIpmiHysteresisSupportFixed )
       temp |= SAHPI_STM_UP_HYSTERESIS | SAHPI_STM_LOW_HYSTERESIS;

  rec.ThresholdDefn.FixedThold = temp;
}


static void
AddSensorEventDataFormat( cIpmiSensor     *sensor,
                          SaHpiSensorRecT &rec )
{
  SaHpiSensorRangeFlagsT temp = 0;

  // Depends on IPMI
  if (rec.Category == SAHPI_EC_THRESHOLD)
       rec.DataFormat.ReadingFormats = SAHPI_SRF_RAW | SAHPI_SRF_INTERPRETED;
  else
       rec.DataFormat.ReadingFormats = SAHPI_SRF_RAW;

  // No info about IsNumeric in IPMI
  rec.DataFormat.IsNumeric     = SAHPI_TRUE;
  rec.DataFormat.SignFormat    = (SaHpiSensorSignFormatT)sensor->AnalogDataFormat();
  rec.DataFormat.BaseUnits     = (SaHpiSensorUnitsT)sensor->BaseUnit();
  rec.DataFormat.ModifierUnits = (SaHpiSensorUnitsT)sensor->ModifierUnit();
  rec.DataFormat.ModifierUse  = (SaHpiSensorModUnitUseT)sensor->ModifierUnitUse();

  rec.DataFormat.FactorsStatic = SAHPI_TRUE;
  // We use first...
  rec.DataFormat.Factors.M_Factor        = (SaHpiInt16T)sensor->M();
  rec.DataFormat.Factors.B_Factor        = (SaHpiInt16T)sensor->B();
  rec.DataFormat.Factors.AccuracyFactor  = (SaHpiUint16T)sensor->Accuracy();
  rec.DataFormat.Factors.ToleranceFactor = (SaHpiUint8T)sensor->Tolerance();
  rec.DataFormat.Factors.ExpA            = (SaHpiUint8T)sensor->AccuracyExp();
  rec.DataFormat.Factors.ExpR            = (SaHpiUint8T)sensor->RExp();
  rec.DataFormat.Factors.ExpB            = (SaHpiUint8T)sensor->BExp();
  rec.DataFormat.Factors.Linearization   = (SaHpiSensorLinearizationT)sensor->Linearization();

  rec.DataFormat.Percentage = (SaHpiBoolT)sensor->Percentage();

  temp |= SAHPI_SRF_MAX | SAHPI_SRF_MIN;
  rec.DataFormat.Range.Max.ValuesPresent = SAHPI_SRF_RAW;
  rec.DataFormat.Range.Max.Raw           = (SaHpiUint32T)sensor->SensorMax();

  rec.DataFormat.Range.Min.ValuesPresent = SAHPI_SRF_RAW;
  rec.DataFormat.Range.Max.Raw           = (SaHpiUint32T)sensor->SensorMin();

  if ( sensor->NominalReadingSpecified() )
     {
       rec.DataFormat.Range.Nominal.ValuesPresent = SAHPI_SRF_RAW;
       rec.DataFormat.Range.Nominal.Raw           = (SaHpiUint32T)sensor->NominalReading();
       temp |= SAHPI_SRF_NOMINAL;
     }

  if ( sensor->NormalMaxSpecified() )
     {
       rec.DataFormat.Range.NormalMax.ValuesPresent = SAHPI_SRF_RAW;
       rec.DataFormat.Range.NormalMax.Raw           = (SaHpiUint32T)sensor->NormalMax();
       temp |= SAHPI_SRF_NORMAL_MAX;
     }

  if ( sensor->NormalMinSpecified() )
     {
       rec.DataFormat.Range.NormalMin.ValuesPresent = SAHPI_SRF_RAW;
       rec.DataFormat.Range.NormalMin.Raw           = (SaHpiUint32T)sensor->NormalMin();
       temp |= SAHPI_SRF_NORMAL_MIN;
     }

  rec.DataFormat.Range.Flags = temp;
}


static void
AddSensorEventSensorRec( cIpmiSensor *sensor,
                         SaHpiSensorRecT &rec )
{
  rec.Num = sensor->Num();
  rec.Type = (SaHpiSensorTypeT)sensor->SensorType();
  rec.Category = (SaHpiEventCategoryT)sensor->EventReadingType();
  rec.EventCtrl = (SaHpiSensorEventCtrlT)sensor->EventSupport();

  rec.Events = 0xffff;
  rec.Ignore = (SaHpiBoolT)sensor->IgnoreIfNoEntity();

  AddSensorEventDataFormat( sensor, rec );
  AddSensorEventThresholds( sensor, rec );

  // We do not care about oem.
  rec.Oem = 0;
}


static void
AddSensorEventRdr( cIpmiSensor *sensor,
                   SaHpiRdrT   &rdr,
                   const SaHpiRptEntryT &resource )
{
  rdr.RecordId = 0;
  rdr.RdrType = SAHPI_SENSOR_RDR;
  rdr.Entity = resource.ResourceEntity;
  
  AddSensorEventSensorRec( sensor, rdr.RdrTypeUnion.SensorRec );

  char	name[32];
  memset( name,'\0',32 );
  sensor->GetId( name, 32 );
  rdr.IdString.DataType = SAHPI_TL_TYPE_ASCII6;
  rdr.IdString.Language = SAHPI_LANG_ENGLISH;
  rdr.IdString.DataLength = 32;

  memcpy( rdr.IdString.Data,name, strlen( name ) + 1 );
}


static void
AddSensorEvent( cIpmiEntity *ent,
                cIpmiSensor *sensor,
                const SaHpiRptEntryT &resource )
{
  struct oh_event *e;

  e = (oh_event *)g_malloc0( sizeof( struct oh_event ) );

  if ( !e )
     {
       IpmiLog( "Out of space !\n" );   
       return;
     }

  memset( e, 0, sizeof( struct oh_event ) );

  e->type                   = oh_event::OH_ET_RDR;

  AddSensorEventRdr( sensor, // id, instance,
                     e->u.rdr_event.rdr, resource );

  //  SaHpiResourceIdT rid = oh_uid_lookup( &e->u.rdr_event.rdr.Entity );
  int rv = oh_add_rdr( ent->Domain()->GetHandler()->rptcache,
                       resource.ResourceId,
                       &e->u.rdr_event.rdr, sensor, 1 );

  assert( rv == 0 );

  // assign the hpi record id to sensor, so we can find
  // the rdr for a given sensor.
  // the id comes from oh_add_rdr.
  sensor->m_record_id = e->u.rdr_event.rdr.RecordId;

  ent->Domain()->AddHpiEvent( e );
}


void
cIpmi::IfSensorAdd( cIpmiEntity *ent, cIpmiSensor *sensor )
{
  IpmiLog( "adding sensor " );
  sensor->Log();

  dbg( "adding sensor %d.%d (%s) %02x: %s",
       ent->EntityId(), ent->EntityInstance(),
       ent->EntityIdString(), sensor->Num(), sensor->Id() );

  // find resource
  SaHpiRptEntryT *resource = ent->Domain()->FindResource( ent->m_resource_id );

  if ( !resource )
     {
       assert( 0 );
       return;
     }

  // update resource capabilities if nessesary
  bool hot_swap_sensor = false;

  if ( sensor->SensorType() == eIpmiSensorTypeAtcaHotSwap )
       hot_swap_sensor = true;

  if (    ( hot_swap_sensor &&
            !(resource->ResourceCapabilities & SAHPI_CAPABILITY_MANAGED_HOTSWAP ) )
       || !(resource->ResourceCapabilities & SAHPI_CAPABILITY_RDR)
       || !(resource->ResourceCapabilities & SAHPI_CAPABILITY_SENSOR ) )
     {
       // update resource
       resource->ResourceCapabilities |= SAHPI_CAPABILITY_RDR|SAHPI_CAPABILITY_SENSOR;

       if ( hot_swap_sensor )
          {
            resource->ResourceCapabilities |= SAHPI_CAPABILITY_MANAGED_HOTSWAP;
            dbg( "adding hotswap capabilities" );
          }

       struct oh_event *e = (struct oh_event *)g_malloc0( sizeof( struct oh_event ) );

       if ( !e )
          {
            IpmiLog( "Out of space !\n" );
            return;
          }

       memset( e, 0, sizeof( struct oh_event ) );
       e->type               = oh_event::OH_ET_RESOURCE;
       e->u.res_event.entry = *resource;

       AddHpiEvent( e );
     }

  AddSensorEvent( ent, sensor, *resource );

  sensor->EventState() = SAHPI_ES_UNSPECIFIED;
}


void
cIpmi::IfSensorRem( cIpmiEntity *ent, cIpmiSensor *sensor )
{
  IpmiLog( "removing sensor %d.%d (%s): %s\n",
           ent->EntityId(), ent->EntityInstance(),
           ent->EntityIdString(), sensor->Id() );

  // find resource
  SaHpiRptEntryT *resource = ent->Domain()->FindResource( ent->m_resource_id );

  if ( !resource )
     {
       assert( 0 );
       return;
     }

  int rv = oh_remove_rdr( ent->Domain()->GetHandler()->rptcache,
                          resource->ResourceId,
                          sensor->m_record_id );

  assert( rv == 0 );

  sensor->m_record_id = 0xfffffff4;
}


void
SetDiscreteSensorMiscEvent( cIpmiEvent *event,
                            SaHpiSensorEventT *e )
{
  tOhEventType type;

  type = (tOhEventType)(event->m_data[10] >> 6);

  if ( type == eEventData2 )
       e->Oem = (SaHpiUint32T)event->m_data[11]; 
  else if ( type == eEventData3 )
       e->SensorSpecific = (SaHpiUint32T)event->m_data[11]; 

  type = (tOhEventType)((event->m_data[10] & 0x30) >> 4);

  if ( type == eEventData2 )
       e->Oem = (SaHpiUint32T)event->m_data[12];
  else if (type == eEventData3 )
       e->SensorSpecific = (SaHpiUint32T)event->m_data[12];
}


void 
SetDiscreteSensorEventState( cIpmiEvent *event,
                             SaHpiEventStateT *state )
{
  tIpmiDiscrete e = (tIpmiDiscrete )(event->m_data[10] & 0x7);

  switch( e )
     {
       case eIpmiTransIdle:
            *state = SAHPI_ES_IDLE;
            break;

       case eIpmiTransActive:
            *state = SAHPI_ES_ACTIVE;
            break;

       case eIpmiTransBusy:
            *state = SAHPI_ES_BUSY;
            break;
     }
}


void
cIpmi::IfSensorDiscreteEvent( cIpmiSensor  *sensor,
                              tIpmiEventDir dir,
                              int           /*offset*/,
                              int           severity,
                              int           /*prev_severity*/,
                              cIpmiEvent   *event )
{
  struct oh_event *e;
  cIpmiEntity     *entity = sensor->GetEntity();
  SaHpiEventStateT state;

  IpmiLog( "reading discrecte event.\n" );

  e = (oh_event *)g_malloc0( sizeof( struct oh_event ) );

  if ( !e )
     {
       IpmiLog( "Out of space !\n");
       return;
     }

  memset(e, 0, sizeof( struct oh_event ) );
  e->type = oh_event::OH_ET_HPI;
  e->u.hpi_event.parent = entity->m_resource_id;
  e->u.hpi_event.id     = sensor->m_record_id;

  e->u.hpi_event.event.Source = 0;
  // Do not find EventType in IPMI
  e->u.hpi_event.event.EventType = SAHPI_ET_SENSOR;
  e->u.hpi_event.event.Timestamp = (SaHpiTimeT)IpmiGetUint32(event->m_data);

  e->u.hpi_event.event.Severity = (SaHpiSeverityT)severity;

  e->u.hpi_event.event.EventDataUnion.SensorEvent.SensorNum = 0;
  e->u.hpi_event.event.EventDataUnion.SensorEvent.SensorType = 
    (SaHpiSensorTypeT)event->m_data[7];
  e->u.hpi_event.event.EventDataUnion.SensorEvent.EventCategory =
    (SaHpiEventCategoryT)event->m_data[9] & 0x7f;
  e->u.hpi_event.event.EventDataUnion.SensorEvent.Assertion = 
    (SaHpiBoolT)!(dir);

  SetDiscreteSensorEventState( event, &state );
  e->u.hpi_event.event.EventDataUnion.SensorEvent.EventState = state;
  
  e->u.hpi_event.event.EventDataUnion.SensorEvent.PreviousState = sensor->EventState();
  sensor->EventState() = state;

  SetDiscreteSensorMiscEvent( event, &e->u.hpi_event.event.EventDataUnion.SensorEvent );

  AddHpiEvent( e );
}


void
SetThresholedSensorEventState( tIpmiThresh threshold,
                               tIpmiEventDir dir,
                               tIpmiEventValueDir high_low,
                               SaHpiSensorEventT *event,
                               SaHpiSeverityT *severity )
{
  if (    ( dir == eIpmiAssertion   && high_low == eIpmiGoingHigh )
       || ( dir == eIpmiDeassertion && high_low == eIpmiGoingLow  ) ) 
       event->Assertion = SAHPI_TRUE;
  else if (   ( dir == eIpmiAssertion   && high_low == eIpmiGoingLow )
           || ( dir == eIpmiDeassertion && high_low == eIpmiGoingHigh ) )
       event->Assertion = SAHPI_FALSE;

  switch( threshold )
     {
       case eIpmiLowerNonCritical:
            event->EventState = SAHPI_ES_LOWER_MINOR;
            *severity = SAHPI_MINOR;
            break;

       case eIpmiLowerCritical:
            event->EventState = SAHPI_ES_LOWER_MAJOR;
            *severity = SAHPI_MAJOR;
            break;

       case eIpmiLowerNonRecoverable:
            event->EventState = SAHPI_ES_LOWER_CRIT;
            *severity = SAHPI_CRITICAL;
            break;

       case eIpmiUpperNonCritical:
            event->EventState = SAHPI_ES_UPPER_MINOR;
            *severity = SAHPI_MINOR;
            break;

       case eIpmiUpperCritical:
            event->EventState = SAHPI_ES_UPPER_MAJOR;
            *severity = SAHPI_MAJOR;
            break;

       case eIpmiUpperNonRecoverable:
            event->EventState = SAHPI_ES_UPPER_CRIT;
            *severity = SAHPI_CRITICAL;
            break;

       default:
            IpmiLog( "Invalid threshold giving");
            event->EventState = SAHPI_ES_UNSPECIFIED;
     }
}


void
SetThresholdsSensorMiscEvent( cIpmiSensor *sensor,
                              cIpmiEvent *event,
                              SaHpiSensorEventT	*e )
{
  unsigned int type;

  // byte 2
  type = event->m_data[10] >> 6;

  if ( type == eEventData1 )
     {
       e->TriggerReading.ValuesPresent = SAHPI_SRF_RAW;
       e->TriggerReading.Raw = (SaHpiUint32T)event->m_data[11];

       double value;
       int rv = sensor->ConvertFromRaw( event->m_data[11], value );

       if ( !rv )
          {
            e->TriggerReading.ValuesPresent |= SAHPI_SRF_INTERPRETED;
            e->TriggerReading.Interpreted.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
            e->TriggerReading.Interpreted.Value.SensorFloat32 = value;
          }
     }
  else if ( type == eEventData2 )
       e->Oem = (SaHpiUint32T)event->m_data[11]; 
  else if ( type == eEventData3 )
       e->SensorSpecific = (SaHpiUint32T)event->m_data[11]; 

  // byte 3
  type = (event->m_data[10] & 0x30) >> 4;

  if ( type == eEventData1 )
     {
       e->TriggerThreshold.ValuesPresent = SAHPI_SRF_RAW;
       e->TriggerThreshold.Raw = (SaHpiUint32T)event->m_data[12];

       double value;
       int rv = sensor->ConvertFromRaw( event->m_data[12], value );

       if ( !rv )
          {
            e->TriggerThreshold.ValuesPresent |= SAHPI_SRF_INTERPRETED;
            e->TriggerThreshold.Interpreted.Type = SAHPI_SENSOR_INTERPRETED_TYPE_FLOAT32;
            e->TriggerThreshold.Interpreted.Value.SensorFloat32 = value;
          }
     }
  else if ( type == eEventData2 )
       e->Oem = (SaHpiUint32T)event->m_data[12];
  else if ( type == eEventData3 )
       e->SensorSpecific = (SaHpiUint32T)event->m_data[12];
}


void
cIpmi::IfSensorThresholdEvent( cIpmiSensor       *sensor,
                               tIpmiEventDir      dir,
                               tIpmiThresh        threshold,
                               tIpmiEventValueDir high_low,
                               cIpmiEvent        *event )
{
  struct oh_event *e;
  cIpmiEntity     *entity = sensor->GetEntity();
  SaHpiSeverityT   severity;

  IpmiLog( "reading threshold event.\n" );

  e = (oh_event *)g_malloc0( sizeof( struct oh_event ) );
  if ( !e )
     {
       IpmiLog( "Out of space !\n" );
       return;
     }

  memset( e, 0, sizeof( struct oh_event ) );
  e->type = oh_event::OH_ET_HPI;
  e->u.hpi_event.parent = entity->m_resource_id;
  e->u.hpi_event.id     = sensor->m_record_id;

  e->u.hpi_event.event.Source = 0;
  // Do not find EventType in IPMI
  e->u.hpi_event.event.EventType = SAHPI_ET_SENSOR;
  e->u.hpi_event.event.Timestamp = (SaHpiTimeT)
    IpmiGetUint32( event->m_data );

  e->u.hpi_event.event.EventDataUnion.SensorEvent.SensorNum = 0;
  e->u.hpi_event.event.EventDataUnion.SensorEvent.SensorType = 
    (SaHpiSensorTypeT)event->m_data[7];
  e->u.hpi_event.event.EventDataUnion.SensorEvent.EventCategory =
    (SaHpiEventCategoryT)event->m_data[9] & 0x7f;

  SetThresholedSensorEventState( threshold, dir, high_low,
                                 &e->u.hpi_event.event.EventDataUnion.SensorEvent,
                                 &severity );
  e->u.hpi_event.event.Severity = severity;

  e->u.hpi_event.event.EventDataUnion.SensorEvent.PreviousState = sensor->EventState();
  sensor->EventState() = e->u.hpi_event.event.EventDataUnion.SensorEvent.EventState;

  SetThresholdsSensorMiscEvent
    (sensor, event, &e->u.hpi_event.event.EventDataUnion.SensorEvent);

  AddHpiEvent( e );
}
