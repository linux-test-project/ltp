/*      -*- linux-c -*-
 *
 * Copyright (c) 2007 Pigeon Point Systems.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *    Anton Pak    <anton.pak@pigeonpoint.com>
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hpi_ui.h>
#include "hpi_cmd.h"

typedef struct {
    SaHpiResourceIdT rptid;
    SaHpiDimiNumT    diminum;
} dimi_block_env_t;

static dimi_block_env_t dimi_block_env;

static ret_code_t get_testnum( SaHpiDimiTestNumT * testnum )
{
    int i, res;

    i = get_int_param( "Test Num: ", &res );
    if ( i != 1 ) {
        printf( "Error!!! Invalid Test Num\n" );
        return HPI_SHELL_PARM_ERROR ;
    }
    *testnum = (SaHpiDimiTestNumT)res;
    return HPI_SHELL_OK;
}


static ret_code_t show_dimi_info( SaHpiSessionIdT sessionId,
                                  SaHpiResourceIdT rptid,
                                  SaHpiDimiNumT diminum ) 
{
    SaErrorT rv;
    SaHpiDimiInfoT info;

    rv = saHpiDimiInfoGet( sessionId, rptid, diminum, &info );
    if ( rv != SA_OK ) {
        printf( "ERROR!!! saHpiDimiInfoGet: %s\n", oh_lookup_error( rv ) );
        return HPI_SHELL_CMD_ERROR;
    }

    printf( "DIMI number: %d\n", (int)diminum );
    printf( "    Number of tests: %d\n", (int)info.NumberOfTests );
    printf( "    Test number update counter: %d\n", (int)info.TestNumUpdateCounter );

    return HPI_SHELL_OK;
}

static ret_code_t show_test_info( SaHpiSessionIdT sessionId,
                                  SaHpiResourceIdT rptid,
                                  SaHpiDimiNumT diminum,
                                  SaHpiDimiTestNumT testnum
                                ) 
{
    SaErrorT rv;
    SaHpiDimiTestT test;
    oh_big_textbuffer bigtmpbuf;
    SaHpiTextBufferT tmpbuf; 
    int i;
    
    rv = saHpiDimiTestInfoGet( sessionId, rptid, diminum, testnum, &test );
    if ( rv != SA_OK ) {
        printf( "ERROR!!! saHpiDimiTestInfoGet: %s\n", oh_lookup_error( rv ) );
        return HPI_SHELL_CMD_ERROR;
    }

    printf( "Test number: %d\n", (int)testnum );
    print_text_buffer_text( "    Name: \"", &test.TestName, NULL, ui_print );
    printf( "\"\n" );

    printf( "    Service Impact: %s", oh_lookup_dimitestserviceimpact( test.ServiceImpact ) );
    printf( "\n" );
    
    printf( "    Affected Entities:\n" );
    for ( i = 0 ; i < SAHPI_DIMITEST_MAX_ENTITIESIMPACTED; ++i ) {
        const SaHpiDimiTestAffectedEntityT * ei = &(test.EntitiesImpacted[i]);

        // Trick suggested to point unset entity pathes
        if ( ei->EntityImpacted.Entry[0].EntityType == SAHPI_ENT_UNSPECIFIED ) {
            break;
        }
        oh_decode_entitypath( &(ei->EntityImpacted), &bigtmpbuf);
        printf( "        %s: %s\n",
                bigtmpbuf.Data,
                oh_lookup_dimitestserviceimpact( ei->ServiceImpact ) 
              );
    }

    if ( test.NeedServiceOS == SAHPI_TRUE ) {
        print_text_buffer_text( "    Needed Service OS: ", &test.ServiceOS, NULL, ui_print );
        printf( "\n" );
    }

    printf( "    Expected Run Duration: %lld nsec\n", test.ExpectedRunDuration );

    oh_decode_dimitestcapabilities( test.TestCapabilities, &tmpbuf );
    printf( "    Test capabilities: %s\n", tmpbuf.Data );

    printf( "    Test parameters:\n" );
    for ( i = 0; i < SAHPI_DIMITEST_MAX_PARAMETERS; ++i ) {
        const SaHpiDimiTestParamsDefinitionT * param = &(test.TestParameters[i]);

        // Trick suggested to point unused params
        if ( param->ParamName[0] == '\0' ) {
            break;
        }
        printf( "        Paramerer %d:\n", i );
        printf( "            Name: \"%s\"\n", param->ParamName );
        printf( "            Info: %s\n", param->ParamInfo.Data );
        switch ( param->ParamType ) {
            case SAHPI_DIMITEST_PARAM_TYPE_BOOLEAN:
                printf( "            Type: boolean\n" );
                printf( "            Default value: %s\n",
                        param->DefaultParam.parambool == SAHPI_TRUE ? "true" : "false" );
                break;
            case SAHPI_DIMITEST_PARAM_TYPE_INT32:
                printf( "            Type: int32\n" );
                printf( "            Min value: %d\n", param->MinValue.IntValue );
                printf( "            Max value: %d\n", param->MaxValue.IntValue );
                printf( "            Default value: %d\n", param->DefaultParam.paramint );
                break;
            case SAHPI_DIMITEST_PARAM_TYPE_FLOAT64:
                printf( "            Type: float64\n" );
                printf( "            Min value: %f\n", param->MinValue.FloatValue );
                printf( "            Max value: %f\n", param->MaxValue.FloatValue );
                printf( "            Default value: %f\n", param->DefaultParam.paramfloat );
                break;
            case SAHPI_DIMITEST_PARAM_TYPE_TEXT:
                printf( "            Type: text\n" );
                printf( "            Default value: %s\n", param->DefaultParam.paramtext.Data );
                break;
            default:
                printf( "            Type: unknown\n" );

        }
    }

    return HPI_SHELL_OK;
}

static ret_code_t show_test_readiness( SaHpiSessionIdT sessionId,
                                       SaHpiResourceIdT rptid,
                                       SaHpiDimiNumT diminum,
                                       SaHpiDimiTestNumT testnum
                                     ) 
{
    SaErrorT rv;
    SaHpiDimiReadyT ready;

    rv = saHpiDimiTestReadinessGet( sessionId, rptid, diminum, testnum, &ready );
    if ( rv != SA_OK ) {
        printf( "ERROR!!! saHpiDimiTestReadinessGet: %s\n", oh_lookup_error( rv ) );
        return HPI_SHELL_CMD_ERROR;
    }
    switch ( ready ) {
        case SAHPI_DIMI_READY:
            printf( "DIMI is ready to run test %d\n", testnum );
            break;
        case SAHPI_DIMI_WRONG_STATE:
            printf( "DIMI is in the wrong state to run test %d\n", testnum );
            break;
        case SAHPI_DIMI_BUSY:
            printf( "DIMI cannot start test %d at this time.\n", testnum );
            break;
        default:
            printf( "Unknown test readiness state(%d).\n", (int)ready );
    }
    return HPI_SHELL_OK;
}

static ret_code_t start_test( SaHpiSessionIdT sessionId,
                              SaHpiResourceIdT rptid,
                              SaHpiDimiNumT diminum,
                              SaHpiDimiTestNumT testnum
                            ) 
{
    SaErrorT rv;

    rv = saHpiDimiTestStart( sessionId, rptid, diminum, testnum, 0, NULL );  
    if ( rv != SA_OK ) {
        printf( "ERROR!!! saHpiDimiTestStart: %s\n", oh_lookup_error( rv ) );
        return HPI_SHELL_CMD_ERROR;
    }

    return HPI_SHELL_OK;
}

static ret_code_t cancel_test( SaHpiSessionIdT sessionId,
                               SaHpiResourceIdT rptid,
                               SaHpiDimiNumT diminum,
                               SaHpiDimiTestNumT testnum
                             ) 
{
    SaErrorT rv;

    rv = saHpiDimiTestCancel( sessionId, rptid, diminum, testnum );
    if ( rv != SA_OK ) {
        printf( "ERROR!!! saHpiDimiTestCancel: %s\n", oh_lookup_error( rv ) );
        return HPI_SHELL_CMD_ERROR;
    }
    return HPI_SHELL_OK;
}

static ret_code_t show_test_status( SaHpiSessionIdT sessionId,
                                    SaHpiResourceIdT rptid,
                                    SaHpiDimiNumT diminum,
                                    SaHpiDimiTestNumT testnum
                                  ) 
{
    SaErrorT rv;
    SaHpiDimiTestPercentCompletedT percent_completed;
    SaHpiDimiTestRunStatusT runstatus;

    rv = saHpiDimiTestStatusGet( sessionId, rptid, diminum, testnum,
                                 &percent_completed, &runstatus
                               );
    if ( rv != SA_OK ) {
        printf( "ERROR!!! saHpiDimiTestStatusGet: %s\n", oh_lookup_error( rv ) );
        return HPI_SHELL_CMD_ERROR;
    }

    printf( "Test number: %d\n", (int)testnum );
    printf( "    Status: %s\n", oh_lookup_dimitestrunstatus( runstatus ) );
    if ( percent_completed != 0xff && runstatus == SAHPI_DIMITEST_STATUS_RUNNING ) {
        printf( "    Percent completed: %d\n", percent_completed );
    }

    return HPI_SHELL_OK;
}

static ret_code_t show_test_results( SaHpiSessionIdT sessionId,
                                     SaHpiResourceIdT rptid,
                                     SaHpiDimiNumT diminum,
                                     SaHpiDimiTestNumT testnum
                                   ) 
{
    SaErrorT rv;
    SaHpiDimiTestResultsT results;
    char date[30];
    

    rv = saHpiDimiTestResultsGet( sessionId, rptid, diminum, testnum, &results );
    if ( rv != SA_OK ) {
        printf( "ERROR!!! saHpiDimiTestResultsGet: %s\n", oh_lookup_error( rv ) );
        return HPI_SHELL_CMD_ERROR;
    }

    printf( "Test number: %d\n", (int)testnum );

    time2str( results.ResultTimeStamp, date, sizeof(date) );
    printf( "    Result timestamp: %s\n", date );
    printf( "    Run duration: %lld nsec\n", results.RunDuration );
    printf( "    Last run status: %s\n", oh_lookup_dimitestrunstatus( results.LastRunStatus ) );

    printf( "    Test error code: " );
    switch( results.TestErrorCode ) {
        case SAHPI_DIMITEST_STATUSERR_NOERR:
            printf( "no Error was generated" );
            break;
        case SAHPI_DIMITEST_STATUSERR_RUNERR:
            printf( "run time error was generated" );
            break;
        case SAHPI_DIMITEST_STATUSERR_UNDEF:
            printf( "undefined error" );
            break;
        default:
            printf( "unknown" );
    }
    printf( "\n" );

    printf( "    Test result %s: \"%s\"\n",
            results.TestResultStringIsURI == SAHPI_TRUE ? "URI" : "string",
            results.TestResultString.Data
          );

    return HPI_SHELL_OK;
}


/*************************************
 * commands
 ************************************/

ret_code_t dimi_block( void )
{
    SaErrorT           rv;
    ret_code_t         ret;
    SaHpiResourceIdT   rptid;
    SaHpiInstrumentIdT rdrnum;
    SaHpiRdrT          rdr;

    ret = ask_rpt( &rptid );
    if ( ret != HPI_SHELL_OK ) {
        return ret;
    };
    ret = ask_rdr( rptid, SAHPI_DIMI_RDR, &rdrnum );
    if ( ret != HPI_SHELL_OK ) {
        return ret;
    }
    rv = saHpiRdrGetByInstrumentId( Domain->sessionId, rptid, SAHPI_DIMI_RDR, rdrnum, &rdr );
    if ( rv != SA_OK ) {
        printf( "ERROR!!! saHpiRdrGetByInstrumentId"
                "(Rpt=%d RdrType=%d Rdr=%d): %s\n",
                rptid, SAHPI_DIMI_RDR, rdrnum,
                oh_lookup_error( rv )
              );
        return HPI_SHELL_CMD_ERROR;
    };

    dimi_block_env.rptid   = rptid;
    dimi_block_env.diminum = rdr.RdrTypeUnion.DimiRec.DimiNum;

    block_type = DIMI_COM;
    for ( ;; ) {
        int res;
        term_def_t * term ;
        char buf[256];

        res = get_new_command( "DIMI block ==> " );
        if ( res == 2 ) {
            unget_term();
            break;
        };
        term = get_next_term();
        if ( term == NULL ) continue;
        snprintf( buf, 256, "%s", term->term );
        if ( ( strcmp( buf, "q" ) == 0) || ( strcmp( buf, "quit" ) == 0 ) ) {
            break;
        }
    }
    block_type = MAIN_COM;
    return HPI_SHELL_OK;
}

ret_code_t dimi_block_info( void )
{
    return show_dimi_info( Domain->sessionId,
                           dimi_block_env.rptid,
                           dimi_block_env.diminum
                         );
}

ret_code_t dimi_block_testinfo( void )
{
    SaHpiDimiTestNumT testnum;

    if ( get_testnum( &testnum) != HPI_SHELL_OK ) {
        return HPI_SHELL_PARM_ERROR;
    }
    return show_test_info( Domain->sessionId,
                           dimi_block_env.rptid,
                           dimi_block_env.diminum,
                           testnum
                         );
}

ret_code_t dimi_block_ready( void )
{
    SaHpiDimiTestNumT testnum;

    if ( get_testnum( &testnum) != HPI_SHELL_OK ) {
        return HPI_SHELL_PARM_ERROR;
    }
    return show_test_readiness( Domain->sessionId,
                                dimi_block_env.rptid,
                                dimi_block_env.diminum,
                                testnum
                              );
}

ret_code_t dimi_block_start( void )
{
    SaHpiDimiTestNumT testnum;

    if ( get_testnum( &testnum) != HPI_SHELL_OK ) {
        return HPI_SHELL_PARM_ERROR;
    }
    return start_test( Domain->sessionId,
                       dimi_block_env.rptid,
                       dimi_block_env.diminum,
                       testnum
                     );
}

ret_code_t dimi_block_cancel( void )
{
    SaHpiDimiTestNumT testnum;

    if ( get_testnum( &testnum) != HPI_SHELL_OK ) {
        return HPI_SHELL_PARM_ERROR;
    }
    return cancel_test( Domain->sessionId,
                        dimi_block_env.rptid,
                        dimi_block_env.diminum,
                        testnum
                      );
}

ret_code_t dimi_block_status( void )
{
    SaHpiDimiTestNumT testnum;

    if ( get_testnum( &testnum) != HPI_SHELL_OK ) {
        return HPI_SHELL_PARM_ERROR;
    }
    return show_test_status( Domain->sessionId,
                             dimi_block_env.rptid,
                             dimi_block_env.diminum,
                             testnum
                           );
}

ret_code_t dimi_block_results( void )
{
    SaHpiDimiTestNumT testnum;

    if ( get_testnum( &testnum) != HPI_SHELL_OK ) {
        return HPI_SHELL_PARM_ERROR;
    }
    return show_test_results( Domain->sessionId,
                              dimi_block_env.rptid,
                              dimi_block_env.diminum,
                              testnum
                            );
}


