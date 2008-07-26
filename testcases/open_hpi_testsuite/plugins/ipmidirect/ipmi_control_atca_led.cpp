/*
 * ipmi_control_atca_led.cpp
 *
 * Copyright (c) 2006 by ESO Technologies.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Pierre Sangouard  <psangouard@eso-tech.com>
 */


extern "C" {
#include "SaHpiAtca.h"
}

#include "ipmi_control_atca_led.h"
#include "ipmi_resource.h"
#include "ipmi_log.h"

cIpmiControlAtcaLed::cIpmiControlAtcaLed( cIpmiMc *mc,
                                          unsigned int num,
                                          unsigned char led_color_capabilities,
                                          unsigned char led_default_local_color,
                                          unsigned char led_default_override_color)
  : cIpmiControl( mc, num, SAHPI_CTRL_LED,
                  SAHPI_CTRL_TYPE_OEM ),
    m_num( num ),
    m_led_color_capabilities( led_color_capabilities ),
    m_led_default_local_color( led_default_local_color ),
    m_led_local_color( led_default_local_color ),
    m_led_default_override_color( led_default_override_color ),
    m_led_override_color( led_default_override_color ),
    m_set_led_state_supported( false )
{
}

cIpmiControlAtcaLed::~cIpmiControlAtcaLed()
{
}

bool
cIpmiControlAtcaLed::IsSupportedColor(AtcaHpiLedColorT hpi_color)
{
    switch(hpi_color)
    {
        case ATCAHPI_LED_COLOR_BLUE:
            return ((m_led_color_capabilities & ATCAHPI_LED_BLUE) != 0);
        case ATCAHPI_LED_COLOR_RED:
            return ((m_led_color_capabilities & ATCAHPI_LED_RED) != 0);
        case ATCAHPI_LED_COLOR_GREEN:
            return ((m_led_color_capabilities & ATCAHPI_LED_GREEN) != 0);
        case ATCAHPI_LED_COLOR_AMBER:
            return ((m_led_color_capabilities & ATCAHPI_LED_AMBER) != 0);
        case ATCAHPI_LED_COLOR_ORANGE:
            return ((m_led_color_capabilities & ATCAHPI_LED_ORANGE) != 0);
        case ATCAHPI_LED_COLOR_WHITE:
            return ((m_led_color_capabilities & ATCAHPI_LED_WHITE) != 0);
        case ATCAHPI_LED_COLOR_NO_CHANGE:
            return true;
        case ATCAHPI_LED_COLOR_USE_DEFAULT:
            return true;
        case ATCAHPI_LED_COLOR_RESERVED:
            return false;
    }

    return false;
}

static unsigned char
hpi_to_atca_color( AtcaHpiLedColorT hpi_color,
                   unsigned char current_color,
                   unsigned char default_color )
{
    switch(hpi_color)
    {
        case ATCAHPI_LED_COLOR_BLUE:
            return 0x01;
        case ATCAHPI_LED_COLOR_RED:
            return 0x02;
        case ATCAHPI_LED_COLOR_GREEN:
            return 0x03;
        case ATCAHPI_LED_COLOR_AMBER:
            return 0x04;
        case ATCAHPI_LED_COLOR_ORANGE:
            return 0x05;
        case ATCAHPI_LED_COLOR_WHITE:
            return 0x06;
        case ATCAHPI_LED_COLOR_NO_CHANGE:
            return current_color;
        case ATCAHPI_LED_COLOR_USE_DEFAULT:
            return default_color;
        case ATCAHPI_LED_COLOR_RESERVED:
            return 0x00;
    }

    return 0x00;
}

static AtcaHpiLedColorT
atca_to_hpi_color( unsigned char atca_color )
{
    switch(atca_color & 0x0F)
    {
        case 0x01:
            return ATCAHPI_LED_COLOR_BLUE;
        case 0x02:
            return ATCAHPI_LED_COLOR_RED;
        case 0x03:
            return ATCAHPI_LED_COLOR_GREEN;
        case 0x04:
            return ATCAHPI_LED_COLOR_AMBER;
        case 0x05:
            return ATCAHPI_LED_COLOR_ORANGE;
        case 0x06:
            return ATCAHPI_LED_COLOR_WHITE;
    }

    return ATCAHPI_LED_COLOR_RESERVED;
}

bool
cIpmiControlAtcaLed::CreateRdr( SaHpiRptEntryT &resource, SaHpiRdrT &rdr )
{
    if ( cIpmiControl::CreateRdr( resource, rdr ) == false )
        return false;

    SaHpiCtrlRecT &rec = rdr.RdrTypeUnion.CtrlRec;
    SaHpiCtrlRecOemT &oem_rec = rec.TypeUnion.Oem;

    oem_rec.MId = ATCAHPI_PICMG_MID;

    oem_rec.ConfigData[0] = m_led_color_capabilities;
    oem_rec.ConfigData[1] = atca_to_hpi_color(m_led_default_local_color);
    oem_rec.ConfigData[2] = atca_to_hpi_color(m_led_default_override_color);

    oem_rec.Default.MId = ATCAHPI_PICMG_MID;
    oem_rec.Default.BodyLength = 6;
    oem_rec.Default.Body[0] = 0;
    oem_rec.Default.Body[1] = 0;
    oem_rec.Default.Body[2] = oem_rec.ConfigData[2];
    oem_rec.Default.Body[3] = oem_rec.ConfigData[1];
    oem_rec.Default.Body[4] = SAHPI_FALSE;
    oem_rec.Default.Body[5] = 0;

    cIpmiMsg ledmsg( eIpmiNetfnPicmg, eIpmiCmdSetFruLedState );
    ledmsg.m_data[0] = dIpmiPicMgId;
    ledmsg.m_data[1] = Resource()->FruId();
    ledmsg.m_data[2] = m_num;
    ledmsg.m_data_len = 6;

    cIpmiMsg ledrsp;

    /*
    There seems that there is an issue with ATCA-HPI mapping in that
    Req 4.5.7.7 says that an LED with a local control state and either an
    Override or Lamp test state should set the Control Mode Read Only status
    to SAHPI_FALSE, however there is no atomic way to check for support
    of the Override State. Therefore, there is no obvious "safe" way to implement
    the spec  as stated, only best alternatives. The approach taken here is to
    set the Control Mode Read Only status to SAHPI_FALSE automatically if the
    LED has a local control state.  This will allow for issuing the Set FRU LED
    state commands, however the unsupported states (Override and/or Lamp Test)
    will return  the specified Completion Code of CCh(invalid date field),
    as stated in the ATCA base spec.
    */
    if ( m_led_default_local_color == 0 )
    {
        rec.DefaultMode.Mode = SAHPI_CTRL_MODE_MANUAL;
        rec.DefaultMode.ReadOnly = SAHPI_TRUE;
        m_set_led_state_supported = false;
        oem_rec.ConfigData[1] = 0;
    }
    else
    {
        rec.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
            rec.DefaultMode.ReadOnly = SAHPI_FALSE;
        m_set_led_state_supported = true;
    }

    rec.WriteOnly = SAHPI_FALSE;

    rec.Oem = ATCAHPI_PICMG_CT_ATCA_LED;

    return true;
}

SaErrorT
cIpmiControlAtcaLed::SetState( const SaHpiCtrlModeT &mode, const SaHpiCtrlStateT &state )
{
    cIpmiMsg ledmsg( eIpmiNetfnPicmg, eIpmiCmdSetFruLedState );
    ledmsg.m_data[0] = dIpmiPicMgId;
    ledmsg.m_data[1] = Resource()->FruId();
    ledmsg.m_data[2] = m_num;
    ledmsg.m_data_len = 6;

    if ( mode == SAHPI_CTRL_MODE_AUTO )
    {
        if ( m_led_default_local_color == 0 )
            return SA_ERR_HPI_READ_ONLY;

        ledmsg.m_data[3] = 0xFC;
        ledmsg.m_data[4] = 0x00;
        ledmsg.m_data[5] = m_led_local_color;
    }
    else if ( mode != SAHPI_CTRL_MODE_MANUAL )
    {
        return SA_ERR_HPI_INVALID_PARAMS;
    }
    else
    {
        if ( m_set_led_state_supported == false )
            return SA_ERR_HPI_READ_ONLY;

        if ( &state == NULL )
            return SA_ERR_HPI_INVALID_PARAMS;

        if ( state.Type != SAHPI_CTRL_TYPE_OEM )
            return SA_ERR_HPI_INVALID_DATA;

        if ( state.StateUnion.Oem.MId != ATCAHPI_PICMG_MID )
            return SA_ERR_HPI_INVALID_DATA;

        if ( state.StateUnion.Oem.BodyLength != 6 )
            return SA_ERR_HPI_INVALID_DATA;

        if ( state.StateUnion.Oem.Body[4] == SAHPI_TRUE )
        {
            if ( state.StateUnion.Oem.Body[5] > 127 )
                return SA_ERR_HPI_INVALID_PARAMS;
        }


        if ( state.StateUnion.Oem.Body[1] == 0xFF )
        {
            if ( state.StateUnion.Oem.Body[0] != 0x00 )
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        else if ( state.StateUnion.Oem.Body[1] == 0x00 )
        {
            if ( state.StateUnion.Oem.Body[0] != 0x00 )
                return SA_ERR_HPI_INVALID_PARAMS;
        }
        else if ( state.StateUnion.Oem.Body[1] > 0xFA )
        {
            return SA_ERR_HPI_INVALID_PARAMS;
        }

        if ( state.StateUnion.Oem.Body[0] > 0xFA )
        {
            return SA_ERR_HPI_INVALID_PARAMS;
        }
        else if ( state.StateUnion.Oem.Body[0] == 0 )
        {
            if (( state.StateUnion.Oem.Body[1] != 0xFF )
                && ( state.StateUnion.Oem.Body[1] != 0x00 ))
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        if ( IsSupportedColor((AtcaHpiLedColorT)state.StateUnion.Oem.Body[2]) == false )
            return SA_ERR_HPI_INVALID_PARAMS;

        if ( m_led_default_local_color != 0 )
        {
            if ( IsSupportedColor((AtcaHpiLedColorT)state.StateUnion.Oem.Body[3]) == false )
                return SA_ERR_HPI_INVALID_PARAMS;
        }

        m_led_override_color = hpi_to_atca_color((AtcaHpiLedColorT)state.StateUnion.Oem.Body[2],
                                                    m_led_override_color,
                                                    m_led_default_override_color);
        if ( m_led_default_local_color != 0 )
            m_led_local_color = hpi_to_atca_color((AtcaHpiLedColorT)state.StateUnion.Oem.Body[3],
                                                    m_led_local_color,
                                                    m_led_default_local_color);

        if ( state.StateUnion.Oem.Body[4] == SAHPI_TRUE )
        {
            ledmsg.m_data[3] = 0xFB;
            ledmsg.m_data[4] = state.StateUnion.Oem.Body[5];
        }
        else
        {
            if ( state.StateUnion.Oem.Body[1] == 0xFF )
            {
                ledmsg.m_data[3] = 0xFF;
                ledmsg.m_data[4] = 0x00;
            }
            else if ( state.StateUnion.Oem.Body[1] == 0x00 )
            {
                ledmsg.m_data[3] = 0x00;
                ledmsg.m_data[4] = 0x00;
            }
            else
            {
                ledmsg.m_data[3] = state.StateUnion.Oem.Body[0];
                ledmsg.m_data[4] = state.StateUnion.Oem.Body[1];
            }
        }

        ledmsg.m_data[5] = m_led_override_color;
    }

    cIpmiMsg ledrsp;

    SaErrorT rv = Resource()->SendCommandReadLock( this, ledmsg, ledrsp );

    if (   rv != SA_OK
        || ledrsp.m_data_len < 2
        || ledrsp.m_data[0] != eIpmiCcOk
        || ledrsp.m_data[1] != dIpmiPicMgId )
    {
        stdlog << "cannot set FRU LED state !\n";
        return (rv != SA_OK) ? rv : SA_ERR_HPI_INVALID_REQUEST;
    }

    return SA_OK;
}

SaErrorT
cIpmiControlAtcaLed::GetState( SaHpiCtrlModeT &mode, SaHpiCtrlStateT &state )
{
    cIpmiMsg ledmsg( eIpmiNetfnPicmg, eIpmiCmdGetFruLedState );
    ledmsg.m_data[0] = dIpmiPicMgId;
    ledmsg.m_data[1] = Resource()->FruId();
    ledmsg.m_data[2] = m_num;
    ledmsg.m_data_len = 3;

    cIpmiMsg ledrsp;

    SaErrorT rv = Resource()->SendCommandReadLock( this, ledmsg, ledrsp );

    if (    rv != SA_OK
        || ledrsp.m_data_len < 6
        || ledrsp.m_data[0] != eIpmiCcOk
        || ledrsp.m_data[1] != dIpmiPicMgId )
    {
        stdlog << "cannot get FRU LED state !\n";
        return (rv != SA_OK) ? rv : SA_ERR_HPI_INVALID_REQUEST;
    }

    if ( &mode != NULL )
    {
        if ( (ledrsp.m_data[2] & 0x06) != 0 )
        {
            mode = SAHPI_CTRL_MODE_MANUAL;
        }
        else
        {
            mode = SAHPI_CTRL_MODE_AUTO;
        }
    }

    if ( &state != NULL)
    {
        state.Type = SAHPI_CTRL_TYPE_OEM;

        state.StateUnion.Oem.MId = ATCAHPI_PICMG_MID;
        state.StateUnion.Oem.BodyLength = 6;

        // Lamp test on
        if ( (ledrsp.m_data[2] & 0x04) != 0 )
        {
            if ( ledrsp.m_data[6] == 0x00 )
            {
                state.StateUnion.Oem.Body[0] = 0;
                state.StateUnion.Oem.Body[1] = 0;
            }
            else if ( ledrsp.m_data[6] == 0xFF )
            {
                state.StateUnion.Oem.Body[0] = 0;
                state.StateUnion.Oem.Body[1] = 0xFF;
            }
            else
            {
                state.StateUnion.Oem.Body[0] = ledrsp.m_data[6];
                state.StateUnion.Oem.Body[1] = ledrsp.m_data[7];
            }
            state.StateUnion.Oem.Body[2] = atca_to_hpi_color(ledrsp.m_data[8]);
            state.StateUnion.Oem.Body[3] = atca_to_hpi_color(ledrsp.m_data[5]);
            state.StateUnion.Oem.Body[4] = SAHPI_TRUE;
            state.StateUnion.Oem.Body[5] = ledrsp.m_data[9];
        }
        // Override state on
        else  if ( (ledrsp.m_data[2] & 0x02) != 0 )
        {
            if ( ledrsp.m_data[6] == 0x00 )
            {
                state.StateUnion.Oem.Body[0] = 0;
                state.StateUnion.Oem.Body[1] = 0;
            }
            else if ( ledrsp.m_data[6] == 0xFF )
            {
                state.StateUnion.Oem.Body[0] = 0;
                state.StateUnion.Oem.Body[1] = 0xFF;
            }
            else
            {
                state.StateUnion.Oem.Body[0] = ledrsp.m_data[6];
                state.StateUnion.Oem.Body[1] = ledrsp.m_data[7];
            }
            state.StateUnion.Oem.Body[2] = atca_to_hpi_color(ledrsp.m_data[8]);
            state.StateUnion.Oem.Body[3] = atca_to_hpi_color(ledrsp.m_data[5]);
            state.StateUnion.Oem.Body[4] = SAHPI_FALSE;
            state.StateUnion.Oem.Body[5] = 0;
        }
        else
        {
            if ( ledrsp.m_data[3] == 0x00 )
            {
                state.StateUnion.Oem.Body[0] = 0;
                state.StateUnion.Oem.Body[1] = 0;
            }
            else if ( ledrsp.m_data[3] == 0xFF )
            {
                state.StateUnion.Oem.Body[0] = 0;
                state.StateUnion.Oem.Body[1] = 0xFF;
            }
            else
            {
                state.StateUnion.Oem.Body[0] = ledrsp.m_data[3];
                state.StateUnion.Oem.Body[1] = ledrsp.m_data[4];
            }
            state.StateUnion.Oem.Body[2] = atca_to_hpi_color(m_led_override_color);
            state.StateUnion.Oem.Body[3] = atca_to_hpi_color(ledrsp.m_data[5]);
            state.StateUnion.Oem.Body[4] = SAHPI_FALSE;
            state.StateUnion.Oem.Body[5] = 0;
        }
    }

    return SA_OK;
}

void
cIpmiControlAtcaLed::Dump( cIpmiLog &dump, const char *name ) const
{
    dump.Begin( "AtcaLedControl", name );

    dump.Entry( "LedNum" ) << m_num << ";\n";

    dump.End();
}
