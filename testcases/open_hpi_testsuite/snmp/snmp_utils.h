/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003, 2004
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      Renier Morales <renier@openhpi.org>
 */

#ifndef SNMP_BC_UTILS_H
#define SNMP_BC_UTILS_H

#include <config.h>
#include <SaHpi.h>

/****** net-snmp config template ******/
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif

#ifdef WIN32
#include <winsock.h>
#endif

/* 
 * This was added to deal with net-snmp 5.1 originally, but
 * it appears to break SuSE, so trying another way.
 * 
 * #define NETSNMP_IMPORT extern
 * #define NETSNMP_INLINE
 * #define RETSIGTYPE void
 * #define NET_SNMP_CONFIG_H
 *
 */

/* Added this to avoid redefinition conflict
 * in opteron based platforms between the linux headers
 * and the net-snmp headers. -- Renier 9/27/04
 */
#ifdef __ssize_t_defined
#define HAVE_SSIZE_T 1
#endif
/**************************************/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/transform_oids.h>

#define MAX_ASN_STR_LEN 300
#define SNMP_BC_MM_BULK_MAX 45
#define SNMP_BC_BULK_DEFAULT 32
#define SNMP_BC_BULK_MIN 16

#define SA_ERR_SNMP_BASE - 10000
#define SA_ERR_SNMP_NOSUCHOBJECT	(SaErrorT)(SA_ERR_SNMP_BASE - SNMP_NOSUCHOBJECT)
#define SA_ERR_SNMP_NOSUCHINSTANCE	(SaErrorT)(SA_ERR_SNMP_BASE - SNMP_NOSUCHINSTANCE)
#define SA_ERR_SNMP_NOSUCHNAME   (SaErrorT)(SA_ERR_SNMP_BASE - SNMP_ERR_NOSUCHNAME)
#define SA_ERR_SNMP_ENDOFMIBVIEW	(SaErrorT)(SA_ERR_SNMP_BASE - SNMP_ENDOFMIBVIEW)
#define SA_ERR_SNMP_ERROR	 	(SaErrorT)(SA_ERR_SNMP_BASE - STAT_ERROR)
#define SA_ERR_SNMP_TIMEOUT             (SaErrorT)(SA_ERR_SNMP_BASE - STAT_TIMEOUT)


#define CHECK_END(a) ((a != SNMP_ENDOFMIBVIEW) &&  \
                      (a != SNMP_NOSUCHOBJECT) &&  \
                      (a != SNMP_NOSUCHINSTANCE))? 1:0 

/* Place-holder for values set and returned by snmp
 */
struct snmp_value {
        u_char type;
        char string[MAX_ASN_STR_LEN];
        size_t str_len;
        //unsigned int str_len;
        long integer;
};

SaErrorT snmp_get(
        void *sessp,
        const char *objid,
        struct snmp_value *value);

SaErrorT snmp_set(
        void *sessp,
        char *objid,
        struct snmp_value value);
		    
SaErrorT snmp_get2(void *sessp, 
		   oid *objid, 
		   size_t objid_len,
		   struct snmp_value *value);

SaErrorT snmp_set2(void *sessp, 
	           oid *objid,
	           size_t objid_len,
                   struct snmp_value *value);

int snmp_getn_bulk( void *sessp, 
		    oid *bulk_objid, 
		    size_t bulk_objid_len,
		    struct snmp_pdu *bulk_pdu, 
		    struct snmp_pdu **bulk_response,
		    int num_repetitions );

void sc_free_pdu(struct snmp_pdu **p);

SaErrorT snmpstat2hpi(int snmpstat);
SaErrorT errstat2hpi(long pdu_errstat);

#endif
