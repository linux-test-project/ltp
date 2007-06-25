/*
 * ipmi_control_fan.cpp
 *
 * Copyright (c) 2004 by FORCE Computers.
 * Copyright (c) 2005-2006 by ESO Technologies.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Thomas Kanngieser <thomas.kanngieser@fci.com>
 *     Pierre Sangouard  <psangouard@eso-tech.com>
 */


#include "ipmi_control_fan.h"
#include "ipmi_resource.h"
#include "ipmi_log.h"


cIpmiControlFan::cIpmiControlFan( cIpmiMc *mc,
                                  unsigned int num,
                                  unsigned int minium_speed_level,
                                  unsigned int maximum_speed_level,
                                  unsigned int default_speed_level,
                                  bool local_control_mode )
  : cIpmiControl( mc, num, SAHPI_CTRL_FAN_SPEED,
                  SAHPI_CTRL_TYPE_ANALOG ),
    m_minimum_speed_level( minium_speed_level ),
    m_maximum_speed_level( maximum_speed_level ),
    m_default_speed_level( default_speed_level ),
    m_local_control_mode( local_control_mode )
{
}


cIpmiControlFan::~cIpmiControlFan()
{
}


bool
cIpmiControlFan::CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr )
{
  if ( cIpmiControl::CreateRdr( resource, rdr ) == false )
       return false;

  SaHpiCtrlRecT &rec = rdr.RdrTypeUnion.CtrlRec;
  SaHpiCtrlRecAnalogT &ana_rec = rec.TypeUnion.Analog;

  ana_rec.Min     = (SaHpiCtrlStateAnalogT)m_minimum_speed_level;
  ana_rec.Max     = (SaHpiCtrlStateAnalogT)m_maximum_speed_level;
  ana_rec.Default = (SaHpiCtrlStateAnalogT)m_default_speed_level;

  rec.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
  rec.DefaultMode.ReadOnly = SAHPI_TRUE;

  rec.WriteOnly = SAHPI_FALSE;

  return true;
}


SaErrorT
cIpmiControlFan::SetState( const SaHpiCtrlModeT &mode, const SaHpiCtrlStateT &state )
{

  if ( mode != SAHPI_CTRL_MODE_AUTO )
  {
    return SA_ERR_HPI_READ_ONLY;
  }

  return SA_OK;
}


SaErrorT
cIpmiControlFan::GetState( SaHpiCtrlModeT &mode, SaHpiCtrlStateT &state )
{
  cIpmiMsg msg( eIpmiNetfnPicmg, eIpmiCmdGetFanLevel );
  msg.m_data[0] = dIpmiPicMgId;
  msg.m_data[1] = Resource()->FruId();
  msg.m_data_len = 2;

  cIpmiMsg rsp;

  SaErrorT rv = Resource()->SendCommandReadLock( this, msg, rsp );

  if (    rv != SA_OK
       || rsp.m_data_len < 3
       || rsp.m_data[0] != eIpmiCcOk
       || rsp.m_data[1] != dIpmiPicMgId )
   {
       stdlog << "cannot send get fan speed !\n";
       return (rv != SA_OK) ? rv : SA_ERR_HPI_INVALID_REQUEST;
   }

  if ( &mode != NULL )
  {
    mode = SAHPI_CTRL_MODE_AUTO;
  }

  if ( &state != NULL)
  {
    state.Type = SAHPI_CTRL_TYPE_ANALOG;

    if (( rsp.m_data_len >= 5 )
        && ( rsp.m_data[4] == 0 ))
    {
        state.StateUnion.Analog = (SaHpiCtrlStateAnalogT)rsp.m_data[2];
    }
    else if ( rsp.m_data_len >= 4 )
    {
        if ( rsp.m_data[2] == dIpmiFanLocalControlMode )
        {
            state.StateUnion.Analog = (SaHpiCtrlStateAnalogT)rsp.m_data[3];
        }
        else
        {
            if ( rsp.m_data[2] > rsp.m_data[3] )
                state.StateUnion.Analog = (SaHpiCtrlStateAnalogT)rsp.m_data[2];
            else
                state.StateUnion.Analog = (SaHpiCtrlStateAnalogT)rsp.m_data[3];
        }
    }
    else
    {
        state.StateUnion.Analog = (SaHpiCtrlStateAnalogT)rsp.m_data[2];
    }
  }

  return SA_OK;
}


void
cIpmiControlFan::Dump( cIpmiLog &dump, const char *name ) const
{
  dump.Begin( "FanControl", name );

  dump.Entry( "ControlNum" ) << m_num << ";\n";
  dump.Entry( "Oem" ) << m_oem << ";\n";
  dump.Entry( "MinimumSpeedLevel" ) << m_minimum_speed_level << ";\n";
  dump.Entry( "MaximumSpeedLevel" ) << m_maximum_speed_level << ";\n";
  dump.Entry( "DefaultSpeedLevel" ) << m_default_speed_level << ";\n";
  dump.Entry( "LocalControlMode" ) << m_local_control_mode << ";\n";

  dump.End();
}
