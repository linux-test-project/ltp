/*      -*- linux-c -*-
 * hpifan.cpp
 *
 * Copyright (c) 2003,2004 by FORCE Computers
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
 *     10/13/2004  kouzmich   porting to HPI B
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <SaHpi.h>
#include <oh_utils.h>
#include <oh_clients.h>

#define OH_SVN_REV "$Revision: 1.6 $"

static int fan_speed = -1;


static int
usage( char *progname )
{
        fprintf( stderr, "usage: %s [-h] [-s fan_speed_level]\n", progname );
        fprintf( stderr, "\t\t -h        help\n" );
        fprintf( stderr, "\t\t -s speed  set fan speed for ALL fans\n" );

        return 1;
}


static void
display_textbuffer( SaHpiTextBufferT string )
{
        int i;
        switch( string.DataType ) {
        case SAHPI_TL_TYPE_BINARY:
                for( i = 0; i < string.DataLength; i++ )
                        printf( "%x", string.Data[i] );
                break;
        case SAHPI_TL_TYPE_BCDPLUS:
                 for( i = 0; i < string.DataLength; i++ )
                        printf( "%c", string.Data[i] );
                break;
       case SAHPI_TL_TYPE_ASCII6:
                 for( i = 0; i < string.DataLength; i++ )
                        printf( "%c", string.Data[i] );
                break;
       case SAHPI_TL_TYPE_UNICODE:
                for( i = 0; i < string.DataLength; i++ )
                        printf( "%c", string.Data[i] );
                break;
	case SAHPI_TL_TYPE_TEXT:
                for( i = 0; i < string.DataLength; i++ )
                        printf( "%c", string.Data[i] );
                break;
        default:
                printf("Invalid string data type=%d", string.DataType );
        }
}


static SaErrorT
get_fan_speed( SaHpiSessionIdT session_id,
               SaHpiResourceIdT resource_id,
               SaHpiCtrlNumT ctrl_num,
               SaHpiCtrlStateAnalogT *speed,
	       SaHpiCtrlModeT *mode )
{
        SaHpiCtrlStateT state;

        SaErrorT rv = saHpiControlGet( session_id, resource_id, ctrl_num, mode, &state );

        if ( rv != SA_OK ) {
                fprintf( stderr, "cannot get fan state: %s!\n", oh_lookup_error( rv ) );
                return rv;
        }

        if ( state.Type != SAHPI_CTRL_TYPE_ANALOG ) {
                fprintf( stderr, "cannot handle non analog fan state !\n" );
                return SA_ERR_HPI_ERROR;
        }

        *speed = state.StateUnion.Analog;

        return SA_OK;
}


static SaErrorT
set_fan_speed( SaHpiSessionIdT session_id,
               SaHpiResourceIdT resource_id,
               SaHpiCtrlNumT ctrl_num,
               SaHpiCtrlStateAnalogT speed,
	       SaHpiCtrlModeT mode )
{
        SaErrorT rv;
        SaHpiCtrlStateT state;
        state.Type = SAHPI_CTRL_TYPE_ANALOG;
        state.StateUnion.Analog = speed;

        rv = saHpiControlSet( session_id, resource_id, ctrl_num, mode, &state );

        if ( rv != SA_OK ) {
                fprintf( stderr, "cannot set fan state: %s!\n", oh_lookup_error( rv ) );
                return rv;
        }

        return SA_OK;
}


static int
do_fan( SaHpiSessionIdT session_id, SaHpiResourceIdT resource_id,
        SaHpiRdrT *rdr )
{
        SaErrorT rv;
        SaHpiCtrlRecT *ctrl_rec = &rdr->RdrTypeUnion.CtrlRec;
	SaHpiCtrlModeT ctrl_mode;

        printf( "\tfan: num %d, id ", ctrl_rec->Num );
        display_textbuffer( rdr->IdString );
        printf( "\n" );

        if ( ctrl_rec->Type != SAHPI_CTRL_TYPE_ANALOG ) {
                fprintf( stderr, "cannot handle non analog fan controls !\n" );
                return 0;
        }

        printf( "\t\tmin       %d\n", ctrl_rec->TypeUnion.Analog.Min );
        printf( "\t\tmax       %d\n", ctrl_rec->TypeUnion.Analog.Max );
        printf( "\t\tdefault   %d\n", ctrl_rec->TypeUnion.Analog.Default );

        SaHpiCtrlStateAnalogT speed;

        rv = get_fan_speed( session_id, resource_id, ctrl_rec->Num, &speed, &ctrl_mode);

        if ( rv != SA_OK )
                return 0;

        printf( "\t\tcurrent   %d\n", speed );

        if ( fan_speed == -1 )
                return 0;
  
        if (    fan_speed < ctrl_rec->TypeUnion.Analog.Min 
                || fan_speed > ctrl_rec->TypeUnion.Analog.Max ) {
                fprintf( stderr, "fan speed %d out of range [%d,%d] !\n",
                         fan_speed, ctrl_rec->TypeUnion.Analog.Min,
                         ctrl_rec->TypeUnion.Analog.Max );
                return 0;
        }

        speed = fan_speed;
        rv = set_fan_speed( session_id, resource_id, ctrl_rec->Num, speed, ctrl_mode);

        if ( rv != SA_OK )
                return 0;
  
        rv = get_fan_speed( session_id, resource_id, ctrl_rec->Num, &speed, &ctrl_mode);

        if ( rv != SA_OK )
                return 0;
  
        printf( "\t\tnew speed %d\n", speed );

        return 0;
}


static int
discover_domain( SaHpiSessionIdT session_id )
{
        /* walk the RPT list */
        SaErrorT rv;
        SaHpiEntryIdT next = SAHPI_FIRST_ENTRY;
        int found = 0;

        do {
                SaHpiRptEntryT entry;
                SaHpiEntryIdT  current = next;

                rv = saHpiRptEntryGet( session_id, current, &next, &entry );

                if ( rv != SA_OK ) {
                        printf( "saHpiRptEntryGet: %s\n", oh_lookup_error( rv ) );
                        return 1;
                }

                // check for control rdr
                if (    !(entry.ResourceCapabilities & SAHPI_CAPABILITY_RDR)
                        || !(entry.ResourceCapabilities & SAHPI_CAPABILITY_CONTROL) )
                        continue;

                /* walk the RDR list for this RPT entry */
                SaHpiEntryIdT next_rdr = SAHPI_FIRST_ENTRY;
                SaHpiResourceIdT resource_id = entry.ResourceId;
                int epath_out = 1;

                do {
                        SaHpiEntryIdT current_rdr = next_rdr;
                        SaHpiRdrT     rdr;

                        rv = saHpiRdrGet( session_id, resource_id, current_rdr, 
                                          &next_rdr, &rdr );
           
                        if ( rv != SA_OK ) {
                                printf( "saHpiRdrGet: %s\n", oh_lookup_error( rv ) );
                                return 1;
                        }

                        // check for control
                        if ( rdr.RdrType != SAHPI_CTRL_RDR )
                                continue;

                        // check for fan
                        if ( rdr.RdrTypeUnion.CtrlRec.OutputType != SAHPI_CTRL_FAN_SPEED )
                                continue;

                        if ( epath_out ) {
				oh_print_ep(&entry.ResourceEntity, 0);
                                epath_out = 0;
                        }

                        do_fan( session_id, resource_id, &rdr );
                        found++;
                } while( next_rdr != SAHPI_LAST_ENTRY );
        } while( next != SAHPI_LAST_ENTRY );

        if ( found == 0 )
                printf( "no fans found.\n" );

        return 0;
}


int
main( int argc, char *argv[] )
{
        int c;
        int help = 0;
        SaErrorT rv;

        oh_prog_version(argv[0], OH_SVN_REV);

        while( (c = getopt( argc, argv,"hs:") ) != -1 )
                switch( c ) {
                case 'h': 
                        help = 1;
                        break;

                case 's':
                        fan_speed = atoi( optarg );
                        break;
                 
                default:
                        fprintf( stderr, "unknown option %s !\n",
                                 argv[optind] );
                        help = 1;
                }
  
        if ( help )
                return usage(argv[0]);

        SaHpiSessionIdT sessionid;
        rv = saHpiSessionOpen( SAHPI_UNSPECIFIED_DOMAIN_ID, &sessionid, 0 );

        if ( rv != SA_OK ) {
                printf( "saHpiSessionOpen: %s\n", oh_lookup_error( rv ) );
                return 1;
        }

        rv = saHpiDiscover( sessionid );

        if ( rv != SA_OK ) {
                printf( "saHpiDiscover: %s\n", oh_lookup_error( rv ) );
                return 1;
        }

        int rc = discover_domain( sessionid );

        rv = saHpiSessionClose( sessionid );

        if ( rv != SA_OK )
                printf( "saHpiSessionClose: %s\n", oh_lookup_error( rv ) );

        return  rc;
}
