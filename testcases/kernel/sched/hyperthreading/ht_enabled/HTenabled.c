/***************************************************************************
                          HTenabled.c  -  description
                             -------------------
    email                : sonic,zhang@intel.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "test.h"
#include "HTutils.h"

char *TCID = "ht_enable";
int TST_TOTAL = 1;

int main(int argc, char *argv[])
{
	tst_resm(TINFO, "Begin: HyperThreading Enabled");

#ifndef ARCH_i386
	tst_brkm(TCONF, NULL, "This test suite can only excute on i386 architecture.");
#else
	if(is_cmdline_para("noht"))
          {
		tst_resm(TINFO, "The kernel boot paramter 'noht' is set.");
		switch(check_ht_capability())
		{
		case 0:
			tst_resm(TFAIL, "HT is enabled.");
			break;
		case 1:
			tst_resm(TPASS, "HT is dinabled.");
			break;
		case 2:
			tst_brkm(TCONF, NULL, "This processor does not support HT.");
			break;
		case 3:
			tst_resm(TFAIL, "HT feature is not included in this Linux Kernel.");
			break;
		default:
			tst_resm(TFAIL, "Unknown reason.");
		}
	}
	else
	{
		tst_resm(TINFO, "The kernel boot paramter 'noht' is not set.");
		switch(check_ht_capability())
		{
		case 0:
			tst_resm(TPASS, "HT is enabled.");
			break;
		case 1:
			tst_resm(TFAIL, "HT is not enabled.");
			break;
		case 2:
			tst_brkm(TCONF, NULL, "This processor does not support HT.");
			break;
		case 3:
			tst_resm(TFAIL, "HT feature is not included in this Linux Kernel.");
			break;
		default:
			tst_resm(TFAIL, "Unknown reason.");
		}
	}
#endif

	tst_resm(TINFO, "End: HyperThreading Enabled");

	return 0;
}

