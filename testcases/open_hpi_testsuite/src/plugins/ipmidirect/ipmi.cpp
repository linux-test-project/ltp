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

#include <netdb.h>
#include <errno.h>
#include <assert.h>

#include "ipmi.h"
#include "ipmi_con_lan.h"
#include "ipmi_con_smi.h"
#include "ipmi_utils.h"


static cIpmi *
VerifyIpmi( void *hnd )
{
  assert( hnd );

  oh_handler_state *handler = (oh_handler_state *)hnd;
  cIpmi *ipmi = (cIpmi *)handler->data;

  if ( !ipmi )
     {
       assert( 0 );
       return 0;
     }

  if ( !ipmi->CheckMagic() )
     {
       assert( 0 );
       return 0;
     }

  if ( !ipmi->CheckHandler( handler ) )
     {
       assert( 0 );
       return 0;
     }

  return ipmi;
}


static cIpmiSensor *
VerifySensorAndEnter( void *hnd, SaHpiResourceIdT rid, SaHpiSensorNumT num,
                      cIpmi *&ipmi )
{
  ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       assert( 0 );
       return 0;
     }

  ipmi->IfEnter();

  SaHpiRdrT *rdr = oh_get_rdr_by_type( ipmi->GetHandler()->rptcache,
                                       rid, SAHPI_SENSOR_RDR, num );
  if ( !rdr )
     {
       ipmi->IfLeave();
       return 0;
     }

  cIpmiSensor *sensor = (cIpmiSensor *)oh_get_rdr_data( ipmi->GetHandler()->rptcache, 
                                                        rid, rdr->RecordId );
  assert( sensor );

  if ( !ipmi->VerifySensor( sensor ) )
     {
       ipmi->IfLeave();
       return 0;
     }

  return sensor;
}


static cIpmiFru *
VerifyFruAndEnter( void *hnd, SaHpiResourceIdT rid, SaHpiEirIdT num,
		   cIpmi *&ipmi )
{
  ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       assert( 0 );
       return 0;
     }

  ipmi->IfEnter();

  SaHpiRdrT *rdr = oh_get_rdr_by_type( ipmi->GetHandler()->rptcache,
                                       rid, SAHPI_INVENTORY_RDR, num );
  if ( !rdr )
     {
       ipmi->IfLeave();
       return 0;
     }

  cIpmiFru *fru = (cIpmiFru *)oh_get_rdr_data( ipmi->GetHandler()->rptcache, 
                                               rid, rdr->RecordId );
  assert( fru );

  if ( !ipmi->VerifyFru( fru ) )
     {
       ipmi->IfLeave();
       return 0;
     }

  return fru;
}


static cIpmiEntity *
VerifyEntityAndEnter( void *hnd, SaHpiResourceIdT rid, cIpmi *&ipmi )
{
  ipmi = VerifyIpmi( hnd );

  if ( !ipmi )
     {
       assert( 0 );
       return 0;
     }

  ipmi->IfEnter();

  cIpmiEntity *ent = (cIpmiEntity *)oh_get_resource_data( ipmi->GetHandler()->rptcache, rid );

  if ( !ipmi->VerifyEntity( ent ) )
     {
       ipmi->IfLeave();
       return 0;
     }

  return ent;
}


// ABI Interface functions
static void *
IpmiOpen( GHashTable *handler_config )
{
  // open log
  const char *logfile = 0;
  int   max_logfiles = 10;
  char *tmp;
  int   lp = dIpmiLogPropNone;

  dbg( "IpmiOpen" );

  if ( !handler_config )
     {
       dbg( "No config file provided.....ooops!" );
       return 0;
     }

  logfile = (char *)g_hash_table_lookup( handler_config, "logfile" );
  tmp = (char *)g_hash_table_lookup( handler_config, "logfile_max" );

  if ( tmp )
       max_logfiles = atoi( tmp );

  tmp = (char *)g_hash_table_lookup( handler_config, "logflags" );

  if ( tmp )
     {
       if (    strstr( tmp, "StdOut" )
            || strstr( tmp, "stdout" ) )
            lp |= dIpmiLogStdOut;

       if (    strstr( tmp, "StdError" )
            || strstr( tmp, "stderr" ) )
            lp |= dIpmiLogStdError;

       if (    strstr( tmp, "File" )
            || strstr( tmp, "file" ) )
          {
            lp |= dIpmiLogFile;

            if ( logfile == 0 )
                 logfile = dDefaultLogfile;
          }
     }
  else
     {
       // default
       lp = dIpmiLogStdOut;

       if ( logfile && *logfile )
            lp |= dIpmiLogFile;
     }

  IpmiLogOpen( lp, logfile, max_logfiles );

  // create domain
  cIpmi *ipmi = new cIpmi;

  // allocate handler
  oh_handler_state *handler = (oh_handler_state *)g_malloc0(
                                  sizeof( oh_handler_state ) );

  if ( !handler )
     {
       dbg("Cannot allocate handler");

       delete ipmi;
       return 0;
     }

  handler->data     = ipmi;
  handler->rptcache = (RPTable *)g_malloc0( sizeof( RPTable ) );
  handler->config   = handler_config;

  ipmi->IfEnter();

  ipmi->SetHandler( handler );

  if ( !ipmi->IfOpen( handler_config ) )
     {
       ipmi->IfClose();
       ipmi->IfLeave();

       delete ipmi;

       oh_flush_rpt( handler->rptcache );
       g_free( handler->rptcache );
       g_free( handler );

       return 0;
     }

  ipmi->IfLeave();

  return handler;
}


static void
IpmiClose( void *hnd )
{
  dbg( "IpmiClose" );

  cIpmi *ipmi = VerifyIpmi( hnd );

  ipmi->IfEnter();
  ipmi->IfClose();
  ipmi->IfLeave();

  delete ipmi;

  oh_handler_state *handler = (oh_handler_state *)hnd;
  assert( handler );

  assert( handler->rptcache );
  oh_flush_rpt( handler->rptcache );
  g_free( handler->rptcache );
  g_free( handler );

  // close logfile
  IpmiLogClose();
}


static SaErrorT
IpmiGetEvent( void *hnd, struct oh_event *event, 
              struct timeval *timeout )
{
  cIpmi *ipmi = VerifyIpmi( hnd );
  ipmi->IfEnter();

  SaErrorT rv = ipmi->IfGetEvent( event, *timeout );
  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiDiscoverResources( void *hnd )
{
  cIpmi *ipmi = VerifyIpmi( hnd );

  ipmi->IfEnter();
  SaErrorT rv = ipmi->IfDiscoverResources();
  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetResourceSeverity( void *hnd, SaHpiResourceIdT id, SaHpiSeverityT sev )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetResourceSeverity( ent, sev );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetSensorData( void *hnd,
                   SaHpiResourceIdT id,
                   SaHpiSensorNumT num,
                   SaHpiSensorReadingT *data )
{
  cIpmi     *ipmi;

  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  memset( data, 0, sizeof( SaHpiSensorReadingT ) );
  SaErrorT rv = ipmi->IfGetSensorData( sensor, *data );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetSensorThresholds( void                   *hnd,
                         SaHpiResourceIdT        id,
                         SaHpiSensorNumT         num,
                         SaHpiSensorThresholdsT *thres )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetSensorThresholds( sensor, *thres );
  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetSensorThresholds( void *hnd,
                         SaHpiResourceIdT id,
                         SaHpiSensorNumT  num,
                         const SaHpiSensorThresholdsT *thres )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetSensorThresholds( sensor, *thres );

  ipmi->IfLeave();
  
  return rv;
}


static SaErrorT
IpmiGetSensorEventEnables( void *hnd, 
                           SaHpiResourceIdT        id,
                           SaHpiSensorNumT         num,
                           SaHpiSensorEvtEnablesT *enables )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetSensorEventEnables( sensor, *enables );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetSensorEventEnables( void *hnd,
                           SaHpiResourceIdT         id,
                           SaHpiSensorNumT          num,
                           const SaHpiSensorEvtEnablesT *enables )
{
  cIpmi *ipmi;
  cIpmiSensor *sensor = VerifySensorAndEnter( hnd, id, num, ipmi );

  if ( !sensor )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetSensorEventEnables( sensor, *enables );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetInventorySize( void *hnd, SaHpiResourceIdT id,
                      SaHpiEirIdT num,
                      SaHpiUint32T *size )
{
  cIpmi *ipmi = 0;
  cIpmiFru *fru = VerifyFruAndEnter( hnd, id, num, ipmi );

  if ( !fru )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetInventorySize( fru, *size );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetInventoryInfo( void *hnd, SaHpiResourceIdT id,
                      SaHpiEirIdT num,
                      SaHpiInventoryDataT *data )
{
  cIpmi *ipmi = 0;
  cIpmiFru *fru = VerifyFruAndEnter( hnd, id, num, ipmi );

  if ( !fru )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetInventoryInfo( fru, *data );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetSelInfo( void *hnd,
                SaHpiResourceIdT id,
                SaHpiSelInfoT   *info )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent || ent->Sel() == 0 )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetSelInfo( ent->Sel(), *info );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetSelTime( void *hnd, SaHpiResourceIdT id, SaHpiTimeT time )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent || ent->Sel() == 0 )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetSelTime( ent->Sel(), time );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiAddSelEntry( void *hnd, SaHpiResourceIdT id,
                 const SaHpiSelEntryT *Event )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent || ent->Sel() == 0 )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfAddSelEntry( ent->Sel(), *Event );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiDelSelEntry( void *hnd, SaHpiResourceIdT id,
                 SaHpiSelEntryIdT sid )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent || ent->Sel() == 0 )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfDelSelEntry( ent->Sel(), sid );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetSelEntry( void *hnd, SaHpiResourceIdT id,
                 SaHpiSelEntryIdT current,
                 SaHpiSelEntryIdT *prev, SaHpiSelEntryIdT *next,
                 SaHpiSelEntryT *entry )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent || ent->Sel() == 0 )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetSelEntry( ent->Sel(), current, *prev, *next, *entry );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiClearSel( void *hnd, SaHpiResourceIdT id )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent || ent->Sel() == 0 )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfClearSel( ent->Sel() );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetHotswapState( void *hnd, SaHpiResourceIdT id, 
                     SaHpiHsStateT *state )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetHotswapState( ent, *state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetHotswapState( void *hnd, SaHpiResourceIdT id, 
                     SaHpiHsStateT state )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetHotswapState( ent, state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiRequestHotswapAction( void *hnd, SaHpiResourceIdT id, 
                          SaHpiHsActionT act )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfRequestHotswapAction( ent, act );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetPowerState( void *hnd, SaHpiResourceIdT id, 
                   SaHpiHsPowerStateT *state )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetPowerState( ent, *state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetPowerState( void *hnd, SaHpiResourceIdT id, 
                   SaHpiHsPowerStateT state )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetPowerState( ent, state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiGetIndicatorState( void *hnd, SaHpiResourceIdT id, 
                       SaHpiHsIndicatorStateT *state )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetIndicatorState( ent, *state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetIndicatorState( void *hnd, SaHpiResourceIdT id,
                       SaHpiHsIndicatorStateT state )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetIndicatorState( ent, state );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiControlParm( void *hnd,
                 SaHpiResourceIdT id,
                 SaHpiParmActionT act )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfControlParm( ent, act );

  ipmi->IfLeave();

  return rv;  
}


static SaErrorT
IpmiGetResetState( void *hnd, SaHpiResourceIdT id, 
                   SaHpiResetActionT *act )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfGetResetState( ent, *act );

  ipmi->IfLeave();

  return rv;
}


static SaErrorT
IpmiSetResetState( void *hnd,
                   SaHpiResourceIdT id,
                   SaHpiResetActionT act )
{
  cIpmi *ipmi = 0;
  cIpmiEntity *ent = VerifyEntityAndEnter( hnd, id, ipmi );

  if ( !ent )
       return SA_ERR_HPI_NOT_PRESENT;

  SaErrorT rv = ipmi->IfSetResetState( ent, act );

  ipmi->IfLeave();

  return rv;
}


static struct oh_abi_v2 oh_ipmi_plugin;
static bool oh_ipmi_plugin_init = false;


__BEGIN_DECLS
int
get_interface( void **pp, const uuid_t uuid )
{
  if ( uuid_compare( uuid, UUID_OH_ABI_V2 ) != 0 )
     {
       *pp = NULL;
       return -1;
     }

  if ( !oh_ipmi_plugin_init )
     {
       memset( &oh_ipmi_plugin, 0, sizeof(oh_ipmi_plugin) );

       oh_ipmi_plugin.open                     = IpmiOpen;
       oh_ipmi_plugin.close                    = IpmiClose;
       oh_ipmi_plugin.get_event                = IpmiGetEvent;
       oh_ipmi_plugin.discover_resources       = IpmiDiscoverResources;
       oh_ipmi_plugin.set_resource_severity    = IpmiSetResourceSeverity;
       oh_ipmi_plugin.get_sel_info             = IpmiGetSelInfo;
       oh_ipmi_plugin.set_sel_time             = IpmiSetSelTime;
       oh_ipmi_plugin.add_sel_entry            = IpmiAddSelEntry;
       oh_ipmi_plugin.del_sel_entry            = IpmiDelSelEntry;
       oh_ipmi_plugin.get_sel_entry            = IpmiGetSelEntry;
       oh_ipmi_plugin.clear_sel                = IpmiClearSel;
       oh_ipmi_plugin.get_sensor_data          = IpmiGetSensorData;
       oh_ipmi_plugin.get_sensor_thresholds    = IpmiGetSensorThresholds;
       oh_ipmi_plugin.set_sensor_thresholds    = IpmiSetSensorThresholds;
       oh_ipmi_plugin.get_sensor_event_enables = IpmiGetSensorEventEnables;
       oh_ipmi_plugin.set_sensor_event_enables = IpmiSetSensorEventEnables;
       oh_ipmi_plugin.get_inventory_size       = IpmiGetInventorySize;
       oh_ipmi_plugin.get_inventory_info       = IpmiGetInventoryInfo;
       oh_ipmi_plugin.get_hotswap_state        = IpmiGetHotswapState;
       oh_ipmi_plugin.set_hotswap_state        = IpmiSetHotswapState;
       oh_ipmi_plugin.request_hotswap_action   = IpmiRequestHotswapAction;
       oh_ipmi_plugin.get_power_state          = IpmiGetPowerState;
       oh_ipmi_plugin.set_power_state          = IpmiSetPowerState;
       oh_ipmi_plugin.get_indicator_state      = IpmiGetIndicatorState;
       oh_ipmi_plugin.set_indicator_state      = IpmiSetIndicatorState;
       oh_ipmi_plugin.control_parm             = IpmiControlParm;
       oh_ipmi_plugin.get_reset_state          = IpmiGetResetState;
       oh_ipmi_plugin.set_reset_state          = IpmiSetResetState;

       oh_ipmi_plugin_init = true;
     }
 
  *pp = &oh_ipmi_plugin;
  return 0;
}

int ipmidirect_get_interface(void **pp, const uuid_t uuid) __attribute__ ((alias("get_interface")));

__END_DECLS


static unsigned int
GetIntNotNull( GHashTable *handler_config, const char *str, unsigned int def = 0 )
{
  const char *value = (const char *)g_hash_table_lookup(handler_config, str );

  if ( !value )
       return def;
  
  unsigned int v = strtol( value, 0, 0 );
  
  if ( v == 0 )
       return def;
  
  return v;
}


cIpmi::cIpmi()
  : m_magic( dIpmiMagic ),
    m_handler( 0 ),
    m_entity_root( 0 )
{
}


cIpmi::~cIpmi()
{
}


void
cIpmi::SetHandler( oh_handler_state *handler )
{
  assert( m_handler == 0 );
  m_handler = handler;
}


cIpmiCon *
cIpmi::AllocConnection( GHashTable *handler_config )
{
  // default is 5s for IPMI
  unsigned int ipmi_timeout = GetIntNotNull( handler_config, "IpmiConnectionTimeout", 5000 );
  IpmiLog( "AllocConnection: IPMITimeout %d ms.\n", ipmi_timeout );

   // default is 1s for ATCA systems
  unsigned int atca_timeout = GetIntNotNull( handler_config, "AtcaConnectionTimeout", 1000 );
  IpmiLog( "AllocConnection: AtcaTimeout %d ms.\n", atca_timeout );

  // default 8 outstanding messages
  unsigned int max_outstanding = GetIntNotNull( handler_config, "MaxOutstanding", 8 );

  if ( max_outstanding < 1 )
       max_outstanding = 1;
  else if ( max_outstanding > 256 )
       max_outstanding = 256;

  IpmiLog( "AllocConnection: Max Outstanding IPMI messages %d.\n", max_outstanding );

  const char *name = (const char *)g_hash_table_lookup(handler_config, "name");

  if ( !name )
     {
       IpmiLog( "Empty parameter !\n");
       return 0;
     }

  IpmiLog( "IpmiAllocConnection: connection name = '%s'.\n", name );

  if ( !strcmp( name, "lan" ) )
     {
       const char     *addr;
       struct in_addr  lan_addr;
       int             lan_port   = dIpmiConLanStdPort;
       tIpmiAuthType   auth       = eIpmiAuthTypeNone;
       tIpmiPrivilege  priv       = eIpmiPrivilegeAdmin;
       char            user[32]   = "";
       char            passwd[32] = "";
       char           *value;
       struct hostent *ent;

       // Address
       addr = (const char *)g_hash_table_lookup(handler_config, "addr");

       if ( !addr )
          {
            IpmiLog( "TCP/IP address missing in config file !\n" );
            return 0;
          }

       IpmiLog( "IpmiAllocConnection: addr = '%s'.\n", addr );
       ent = gethostbyname( addr );

       if ( !ent )
          {
            IpmiLog( "Unable to resolve IPMI LAN address: %s !\n", addr );
            return 0;
          }

       memcpy( &lan_addr, ent->h_addr_list[0], ent->h_length );
       unsigned int a = *(unsigned int *)ent->h_addr_list[0];

       IpmiLog( "Using host at %d.%d.%d.%d.\n",
                a & 0xff, (a >> 8 ) & 0xff, 
                (a >> 16) & 0xff, (a >> 24) & 0xff );

       // Port
       lan_port = GetIntNotNull( handler_config, "port", 623 );

       IpmiLog( "IpmiAllocConnection: port = %i.\n", lan_port );

       // Authentication type
       value = (char *)g_hash_table_lookup( handler_config, "auth_type" );

       if ( value )
          {
            if ( !strcmp( value, "none" ) )
                 auth = eIpmiAuthTypeNone;
            else if ( !strcmp( value, "straight" ) )
                 auth = eIpmiAuthTypeStraight;
            else if ( !strcmp( value, "md2" ) )
                 auth = eIpmiAuthTypeMd2;
            else if ( !strcmp( value, "md5" ) )
                 auth = eIpmiAuthTypeMd5;
            else
               {
                 IpmiLog( "Invalid IPMI LAN authenication method '%s' !\n", value );
                 return 0;
               }
          }

       IpmiLog( "IpmiAllocConnection: authority: %s(%i).\n", value, auth );

       // Priviledge
       value = (char *)g_hash_table_lookup(handler_config, "auth_level" );

       if ( value )
          {
            if ( !strcmp( value, "callback" ) )
                 priv = eIpmiPrivilegeCallback;
            else if ( !strcmp( value, "user" ) )
                 priv = eIpmiPrivilegeUser;
            else if ( !strcmp( value, "operator" ) )
                 priv = eIpmiPrivilegeOperator;
            else if ( !strcmp( value, "admin" ) )
                 priv = eIpmiPrivilegeAdmin;
            else
               {
                 IpmiLog( "Invalid authentication method '%s' !\n", value );
                 return 0;
               }
          }

       IpmiLog( "IpmiAllocConnection: priviledge = %s(%i).\n", value, priv );

       // User Name
       value = (char *)g_hash_table_lookup( handler_config, "username" ); 

       if ( value )
            strncpy( user, value, 32);

       IpmiLog( "IpmiAllocConnection: user = %s.\n", user );

       // Password
       value = (char *)g_hash_table_lookup( handler_config, "password" );

       if ( value )
            strncpy( passwd, value, 32 );

       IpmiLog( "IpmiAllocConnection: password = %s.\n", user );

       return new cIpmiConLan( ipmi_timeout, atca_timeout, max_outstanding, 
                               lan_addr, lan_port, auth, priv,
                               user, passwd );
     }
  else if ( !strcmp( name, "smi" ) )
     {
       const char *addr = (const char *)g_hash_table_lookup(handler_config, "addr");

       int if_num = 0;
       
       if ( addr )
            if_num = strtol( addr, 0, 10 );

       IpmiLog( "IpmiAllocConnection: interface number = %d.\n", if_num );

       return new cIpmiConSmi( ipmi_timeout, atca_timeout, max_outstanding, if_num );
     }
/*
  else if ( !strcmp( name, "file" ) )
     {
       // filename
       char *file = (char *)g_hash_table_lookup( handler_config, "file");

       IpmiLog( "IpmiAllocConnection: file = %s.\n", file ? file : dIpmiConFileDefault );

       // if file == 0 => use default file
       return new cIpmiConFile( timeout, atca_timeout, file );
     }
*/

  IpmiLog( "Unknown connection type: %s !\n", name );

  return 0;
}


void
cIpmi::AddHpiEvent( oh_event *event )
{
  assert( m_handler );
  m_handler->eventq = g_slist_append( m_handler->eventq, event );
}


const char *
cIpmi::EntityRoot()
{
  return m_entity_root;
}


oh_handler_state *
cIpmi::GetHandler()
{
  return m_handler;
}


SaHpiRptEntryT *
cIpmi::FindResource( SaHpiResourceIdT rid )
{
  assert( m_handler );

  return oh_get_resource_by_id( m_handler->rptcache, rid);
}


void
cIpmi::IfEnter()
{
}


void
cIpmi::IfLeave()
{
  HandleAsyncEvents();
}


bool
cIpmi::GetParams( GHashTable *handler_config )
{
  // get mcs to scan
  char str[100];

  for( unsigned int i = 0; i < 256; i++ )
     {
       sprintf( str, "MC%d", i );

       unsigned int value = GetIntNotNull( handler_config, str );

       if ( value == 0 )
          {
            sprintf( str, "MC0x%02x", i );
            value = GetIntNotNull( handler_config, str );
          }

       if ( value == 0 )
            continue;

       for( unsigned int j = 0; j < dIpmiMaxChannel; j++ )
          {
            if ( (value & ( 1 << j )) == 0 )
                 continue;

            IpmiLog( "GetParam: mc to scan (%x, %x).\n", j, i );

            AddMcToScan( j, i );
          }
     }

  return true;
}


bool
cIpmi::IfOpen( GHashTable *handler_config )
{
  m_entity_root = (const char *)g_hash_table_lookup( handler_config, "entity_root" );

  if ( !m_entity_root )
     {
       dbg( "entity_root is missing in config file" );
       return false;
     }

  cIpmiCon *con = AllocConnection( handler_config );

  if ( !con )
     {
       IpmiLog( "IPMI cannot alloc connection !\n" );
       return false;
     }

  if ( !GetParams( handler_config ) )
     {
       delete con;
       return false;
     }

  bool rv = con->Open();

  if ( rv == false )
     {
       IpmiLog( "IPMI open connection fails !\n" );

       delete con;

       return false;
     }

  if ( !Init( con ) )
     {
       IfClose();
       return false;
     }

  return true;
}


void
cIpmi::IfClose()
{
  Cleanup();

  if ( m_con )
     {
       delete m_con;
       m_con = 0;
     }
}


int
cIpmi::IfGetEvent( oh_event *event, const timeval & /*timeout*/ )
{
  // handle events now
  HandleAsyncEvents();

  if ( g_slist_length( m_handler->eventq ) > 0 )
     {
       memcpy( event, m_handler->eventq->data, sizeof( oh_event ) );
       free( m_handler->eventq->data );
       m_handler->eventq = g_slist_remove_link( m_handler->eventq, m_handler->eventq );
       return 1;
     }

  // no events
  return 0;
}


SaErrorT
cIpmi::IfDiscoverResources()
{
  dbg( "ipmidirect discover_resources");

  if ( !m_is_atca )
     {
       // rescan ipmb bus
       ScanMc( 0, 0x20 );
       return SA_OK;
     }

  return SA_OK;
}


SaErrorT
cIpmi::IfSetResourceSeverity( cIpmiEntity *ent, SaHpiSeverityT sev )
{
  // TODO: add real functionality

  // change severity in plugin cache
  SaHpiRptEntryT *rptentry = oh_get_resource_by_id( ent->Domain()->GetHandler()->rptcache, 
                                                    ent->m_resource_id );
  assert( rptentry );

  rptentry->ResourceSeverity = sev;

  // send update event
  struct oh_event *e = (struct oh_event *)g_malloc0( sizeof( struct oh_event ) );

  if ( !e )
     {
       IpmiLog( "Out of space !\n" );
       return SA_ERR_HPI_OUT_OF_SPACE;
     }

  memset( e, 0, sizeof( struct oh_event ) );
  e->type               = oh_event::OH_ET_RESOURCE;
  e->u.res_event.entry = *rptentry;

  AddHpiEvent( e );

  return SA_OK;
}


SaErrorT
cIpmi::IfControlParm( cIpmiEntity * /*ent*/, SaHpiParmActionT act )
{
  // TODO: implementation
  switch( act )
     {
       case SAHPI_DEFAULT_PARM:
            break;

       case SAHPI_SAVE_PARM:
            break;

       case SAHPI_RESTORE_PARM:
            break;
     }
  
  return SA_OK;
}
