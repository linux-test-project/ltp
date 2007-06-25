/* -*- linux-c -*-
 * 
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
 *     Chris Chia <cchia@users.sf.net>
 */

/**********************************************************************
 * Notes:
 *
 * Test depends on companion input file called sim_test_file, any change
 * in that file needs to be reflected in this test.
 **********************************************************************/

#include <snmp_bc_plugin.h>
#include <sim_resources.h>
#include <sim_init.h>

int main(int argc, char **argv)
{
        SaErrorT err;
        SaHpiSessionIdT sessionid;

        err = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &sessionid, NULL);
        if (err) {
	  printf("  Error! Testcase failed. Line=%d\n", __LINE__);
	  printf("  Received error=%s\n", oh_lookup_error(err));
	  return -1;
        }
	// regular sim_init is part of saHpiSessionOpen, here we close it
	sim_close();
	// env variable OPENHPI_SIMTEST_FILE is now defined in Makefile.am
	// setenv("OPENHPI_SIMTEST_FILE","./sim_test_file", 1);
	// create hash table based on input file
	err = sim_file();
	if (err != SA_OK) {
	  printf(" Error! sim_file failed\n");
	  return -1;
	} 

	/****************** 
	 * End of testcases 
         ******************/

        err = saHpiSessionClose(sessionid);
        if (err) {
	  printf("Error! saHpiSessionClose: err=%d\n", err);
	  return -1;
        }

        return 0;
}
