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

#include <stdio.h>
#include <stdlib.h>

#include "ipmi.h"


static GHashTable *AllocParams( const char **params );
static gboolean RemoveParam( gpointer key, gpointer value, gpointer  user_data );
static void DestryParams( GHashTable *params );


static const char *plugin_params_lan_local[] =
{
  "entity_root", "{SYSTEM_CHASSIS,6}",
  "name",       "lan",
  "addr",       "localhost",
  "port",       "4711",
  "auth_type",  "md5",
  "auth_level", "admin",
  "username",   "kanne",
  "password",   "kanne",
  "logfile",    "log",
  "logfile_max", "1",
  0
};


static const char *plugin_params_lan_remote[] =
{
  "entity_root", "{SYSTEM_CHASSIS,6}",
  "name",       "lan",
  "addr",       "192.168.111.166",
  "port",       "623",
  "auth_type",  "none",
  "auth_level", "admin",
  "username",   "kanne",
  "password",   "kanne",
  "logfile",    "log",
  "logfile_max", "1",
  "IpmiConnectionTimeout", "500000",
  "AtcaConnectionTimeout", "500000",
  0
};


static const char *plugin_params_smi[] =
{
  "entity_root", "{SYSTEM_CHASSIS,6}",
  "name",       "smi",
  "addr",       "0",
  "logfile",    "log",
  "logfile_max", "10",
  0
};


static const char *plugin_params_file[] =
{
  "entity_root", "{SYSTEM_CHASSIS,6}",
  "name",       "file",
  "file",       "ipmi.con",
  "logfile",    "log",
  "logfile_max", "10",
  0
};


struct cPluginParams
{
  const char  *m_name;
  const char **m_params;
};


static cPluginParams plugin_params[] = 
{
  { "lan_local" , plugin_params_lan_local  },
  { "lan_remote", plugin_params_lan_remote },
  { "smi"       , plugin_params_smi        },
  { "file"      , plugin_params_file       },
  { 0, 0 }
};


static const char **
FindConfig( const char *name )
{
  for( int i = 0; plugin_params[i].m_name; i++ )
       if ( !strcasecmp( name, plugin_params[i].m_name ) )
            return plugin_params[i].m_params;

  return 0;
}


static GHashTable *
AllocParams( const char **p )
{
  GHashTable *params = g_hash_table_new( g_str_hash, g_str_equal );

  while( *p )
     {
       const char *pp = *p;
       
       char *v0 = new char[strlen( pp ) + 1];
       strcpy( v0, pp );

       pp = *(p+1);
       char *v1 = new char[strlen( pp ) + 1];
       strcpy( v1, pp );

       g_hash_table_insert( params, v0, v1 );

       p++;
       p++;
     }

  return params;
}


static gboolean 
RemoveParam( gpointer  /*key*/,
             gpointer  /*value*/,
             gpointer  /*user_data*/ )
{
  return 1;
}


static void
DestryParams( GHashTable *params )
{
  g_hash_table_foreach_remove( params, RemoveParam, 0 );
  g_hash_table_destroy( params );
}

/*
static void
TestIpmi( tIpmiCon *con )
{
  tIpmiMsg                 msg;
  tIpmiMsg                 rsp_msg;
  int			   rv;
  tIpmiSystemInterfaceAddr addr;
  unsigned int manufacturer_id;
  unsigned int product_id;

  memset( &addr, 0, sizeof( tIpmiAddr ) );
  
  addr.addr_type = dIpmiSystemInterfaceAddrType;
  addr.channel   = 0xf;
  addr.lun       = 0;

  msg.netfn    = dIpmiAppNetfn;
  msg.cmd      = dIpmiGetDeviceIdCmd;
  msg.data_len = 0;

  rv = IpmiConExecuteCmd( con, (tIpmiAddr *)&addr, sizeof( tIpmiSystemInterfaceAddr ),
                          &msg, &rsp_msg );

  if ( rv )
     {
       printf( "IpmiConExecuteCmd: %d, %s\n", rv, strerror( rv ) );
       return;
     }

  manufacturer_id =    (rsp_msg.data[7]
                        | (rsp_msg.data[8] << 8)
                        | (rsp_msg.data[9] << 16));
  product_id      = rsp_msg.data[10] | (rsp_msg.data[11] << 8);

  printf( "manufacturer = 0x%04x\nproduct = 0x%04x\n",
          manufacturer_id, product_id );
}
*/


void
TestInterface( struct oh_abi_v2 *abi, void *hdl )
{
  abi->discover_resources( hdl );

  int i = 0;
  
  while( i++ < 1000 )
     {
       struct oh_event event;
       struct timeval to = { 0, 0 };

       int rv = abi->get_event( hdl, &event, &to );

       if ( rv )
            printf( "********** read event *********\n" );

       usleep( 100000 );
     }
}


__BEGIN_DECLS
int ipmidirect_get_interface(void **pp, const uuid_t uuid);
__END_DECLS


static int
usage( const char *filename )
{
  fprintf( stderr, "usage: %s config_name\n", filename );
  fprintf( stderr, "\tavailable configs are:\n" );

  for( int i = 0; plugin_params[i].m_name; i++ )
       fprintf( stderr, "\t\t%s\n", plugin_params[i].m_name );

  return 10;
}


int
main( int argc, char *argv[] )
{
  if ( argc > 2 )
       return usage( argv[0] );

  const char *config_name = "lan_local";

  if ( argc == 2 )
       config_name = argv[1];

  const char **config = FindConfig( config_name );

  if ( config == 0 )
       return usage( argv[0] );

  void *dummy;
  struct oh_abi_v2 *abi = 0;
  GHashTable *params;
  void *hdl;
  uuid_t myuid;

  oh_uid_initialize();

  memcpy( &myuid, UUID_OH_ABI_V2, sizeof(myuid) );

  if (    ipmidirect_get_interface( &dummy, myuid ) < 0
       || dummy == 0 )
     {
       printf( "cannot get interface !\n" );
       return 10;
     }

  abi = (struct oh_abi_v2 *) dummy;

  if ( abi->open == 0 )
     {
       printf( "cannot get interface !\n" );
       return 10;
     }
	  
  params = AllocParams( config );

  hdl = abi->open( params );

  if ( hdl == 0 )
     {
       printf( "cannot open plugin !\n" );
       return 10;
     }

  printf( "ready.\n" );
  TestInterface( abi, hdl );

  abi->close( hdl );
  DestryParams( params );

  return 0;
}
