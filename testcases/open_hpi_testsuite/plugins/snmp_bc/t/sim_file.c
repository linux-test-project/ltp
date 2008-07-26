/*      -*- linux-c -*-
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Chris Chia <cchia@users.sf.net>
 *
 * Main function:
 *   Parse a file, either pointed to by the environmental variable OPENHPI_SIMTEST_FILE
 * or default file name "test_file", to build a hash table that will be used by
 * bladecenter/RSA simulator to simulate resource access.
 *
 *   Each line in input file represents an entry in the hash table
 * for example:
 * .1.3.6.1.4.1.2.3.51.2.2.3.1.0 = STRING: "    46% of maximum"
 *    ".1.3.6..." is the key into hash table
 *    STRING and "46% of maximum" are values associated to this key 
 * Sample input file
 *   .1.3.6.1.2.1.1.2.0 = OID: .1.3.6.1.4.1.2.6.158.3
 *   .1.3.6.1.4.1.2.3.51.2.2.3.1.0 = STRING: "    46% of maximum"
 *   .1.3.6.1.4.1.2.3.51.2.2.5.2.6.0 = INTEGER: 0
 *   .1.3.6.1.4.1.2.3.51.2.2.21.8.1.1.9.1 = Hex-STRING: D0 07 F0 D2
 *   .1.3.6.1.4.1.2.3.51.2.2.21.21.1.7.14 = ""
 *   .1.3.6.1.4.1.2.3.51.2.22.5.1.1.3.1 = IpAddress: 9.3.202.71
 *   .1.3.6.1.6.3.11.2.1.1.0 = Counter32: 0
 *   .1.3.6.1.6.3.12.1.1.0 = INTEGER: 0
 *   .1.3.6.1.6.3.12.1.2.1.2.72.111.115.116.49 = Wrong Type (should be OBJECT IDENTIFIER): INTEGER: 1
 *
 *   Input file can be hand made or the output of snmp walk, such as:
 * snmpwalk -v 3 -t 10 -l authNoPriv -A netsnmpv3 -n "" -u $user $host -Of -On .1
 *
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <snmp_utils.h>
#include <sim_resources.h>
#include <snmp_bc_plugin.h>
#include <sim_init.h>

//static void print_entry(gpointer, gpointer, gpointer);   // hash table debug code
//static void print_entry (gpointer key, gpointer val, gpointer data)
//{
//    SnmpMibInfoT* key_val = val;	
//    printf ("\t%s translates to ", (gchar *)key); 
//    if (key_val->type == ASN_INTEGER )	
//        printf (" integer type: %d \n", (int)key_val->value.integer);
//    else if (key_val->type == ASN_OCTET_STR )	
//        printf (" string type: %s \n", key_val->value.string);
//    else printf(" invalid type: %d\n", key_val->type);
//}
	
static void free_hash_data(gpointer key, gpointer value, gpointer user_data);

SaErrorT sim_file()
{
	#define   MAX_STR_LEN  1024
	#define   STD_TOKEN_CNT   4
	FILE     *file;
        char     *file_in;
        char      file_std[] = "./sim_test_file";
	char	  str_in[MAX_STR_LEN];
	gchar	  OID_HDR[] =     ".1.3.6.1.4.1.2.3.51.?";
	gchar	  BC_OID_HDR[] =  ".1.3.6.1.4.1.2.3.51.2";
	gchar	  RSA_OID_HDR[] = ".1.3.6.1.4.1.2.3.51.1";
	gchar	  STR_TYPE[] = "STRING:";
	gchar	  INT_TYPE[] = "INTEGER:";
	gchar	**tokens = NULL;
	gchar	 *tmpstr = NULL;
        const     gchar  *str_delimiter = " ";
	int	  valid, invalid, total, token_cnt, ii;
	int	  rc;
	gboolean  found_plat = FALSE;
	gchar    *key = NULL;
	gchar    *key_exists = NULL;
	SnmpMibInfoT *mibinfo;


	rc = valid = invalid = total = token_cnt = ii = 0;

        file_in = getenv("OPENHPI_SIMTEST_FILE");
        if (!file_in)  file_in = file_std; 
        dbg("file to be tested - %s\n", file_in);      
	file = fopen( file_in, "r");
	if ( file == NULL ) {
		printf("file %s open failed\n", file_in);
		rc = -1;
		goto cleanup;
	}
	clearerr( file );
	sim_hash = g_hash_table_new(g_str_hash, g_str_equal);
        if (sim_hash == NULL) {
        	printf("Cannot allocate simulation hash table");
		rc = -1;
		goto cleanup;
	}
	dbg("---- hash table address %p ----\n", sim_hash);
	while ( !feof(file) )  {  // process each line in file
		dbg("xxx--- hash table size %d ---xxx\n", g_hash_table_size(sim_hash));
		//g_hash_table_foreach (sim_hash, print_entry, NULL);
		if (fgets(str_in, MAX_STR_LEN, file) == NULL)
			dbg("xxx--- Experience problem, check env OPENHPI_SIMTEST_FILE or ./sim_test_file ---xxx\n");
		//dbg("%s", str_in);
		g_strstrip(str_in);
		if (str_in[0] == '\0') {
			//dbg("Skipped a blank line\n");
			continue;
		}
		total++;
		tokens = g_strsplit(str_in, str_delimiter, STD_TOKEN_CNT);
		for (token_cnt=0; tokens[token_cnt]; token_cnt++);
		//dbg("line has %d tokens\n", token_cnt);
		if (token_cnt != STD_TOKEN_CNT) {
			//dbg("Error: tokens (%d) < then expected\n",token_cnt);
                	g_strfreev(tokens);
			invalid++;
			continue;
		}
		if ( found_plat == FALSE ) { // not knowing which platform to test
					     // first see if this is BladeCenter
			tmpstr = strstr(tokens[0], BC_OID_HDR);
			if (tmpstr != tokens[0]) {
					     // then see if it's RSA
				tmpstr = strstr(tokens[0], RSA_OID_HDR);
				if (tmpstr != tokens[0]) {
					     // not a valid platform, skip this line
					//dbg("invalid oid %s\n", tokens[0]);
                			g_strfreev(tokens);
					invalid++;
					continue;
				}
				else {  // we are testing RSA
					found_plat = TRUE;
					strcpy( OID_HDR, RSA_OID_HDR );
				}
			}
			else {          // we are testing BladeCenter
				found_plat = TRUE;
				strcpy( OID_HDR, BC_OID_HDR );
			}
		}
		else {  // platform has been identified, check against ID
			tmpstr = strstr(tokens[0], OID_HDR);
			if (tmpstr != tokens[0]) {
                		g_strfreev(tokens);
				invalid++;
				continue;
			}
		}
		key = g_strdup(tokens[0]);
		//dbg("key = %s\n", key);
		key_exists = g_hash_table_lookup(sim_hash, key);
                if (key_exists) {  // key already processed, skip this line
			dbg("=== oid %s already processed ===\n", key);
			g_free(key);
                	g_strfreev(tokens);
			invalid++;
			continue;
		}
		mibinfo = g_malloc0(sizeof(SnmpMibInfoT));
                if (!mibinfo) {
                        printf("Error: Malloc failed for oid (%s) hash", tmpstr);
			invalid++;
			g_free(key);
			g_strfreev(tokens);
			rc = -1;
                        goto cleanup;
                }
        	if (!g_ascii_strcasecmp(tokens[2], INT_TYPE)) {  // Integer
        		dbg("=== oid %s got a int type: %d\n", key, atoi(tokens[3]));
			mibinfo->value.integer = atoi(tokens[3]);
			mibinfo->type = ASN_INTEGER;
			g_hash_table_insert(sim_hash, key, mibinfo);
                	g_strfreev(tokens);
        		valid++;
        	}
        	else if (!g_ascii_strcasecmp(tokens[2], STR_TYPE)) {  // String
        		// delete quote marks (") at both ends of string
			tmpstr = tokens[3];
			if ( *tmpstr == '\"' ) tmpstr++;
			ii = strlen( tmpstr );
			if (tmpstr[ii -1] == '\"')  tmpstr[ii -1] = '\0';
        		dbg("=== oid %s got a string type: %s\n", key, tmpstr);
			strcpy(mibinfo->value.string, tmpstr);
			mibinfo->type = ASN_OCTET_STR;
			g_hash_table_insert(sim_hash, key, mibinfo);
                	g_strfreev(tokens);
        		valid++;
        	}
        	else {
        		dbg("not a valid type %s\n", tokens[2]);
			g_free(key);
                	g_strfreev(tokens);
			g_free(mibinfo);
			invalid++;
        	}
	}
	dbg("%d out of %d lines in file %s got processed\n", valid, total, file_in);

	fclose( file );
	// g_hash_table_foreach (sim_hash, print_entry, NULL);
	goto done;

cleanup:
	fclose( file );
        g_hash_table_foreach(sim_hash, free_hash_data, NULL);
        g_hash_table_destroy(sim_hash);

done:
	return rc;
}

static void free_hash_data(gpointer key, gpointer value, gpointer user_data)
{
        g_free(key);
        g_free(value);
}

