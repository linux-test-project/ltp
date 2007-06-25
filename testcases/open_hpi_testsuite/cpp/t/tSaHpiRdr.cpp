/*      -*- linux-c -*-
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
 *    W. David Ashley <dashley@us.ibm.com>
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
extern "C"
{
#include <SaHpi.h>
}
#include "oSaHpiRdr.hpp"


int main(int argc, char *argv[]) {
    oSaHpiRdr *ptr1;

    // create the first rdr
    ptr1 = new oSaHpiRdr;
    if (ptr1 == NULL) {
        printf("Error: Unable to create a oSaHpiRdr.\n");
        return -1;
    }

    // print the contents of the first rdr
    fprintf(stdout, "\nSaHpiRdr\n");
    if (ptr1->fprint(stdout, 3)) {
        printf("Error: Unable to print the buffer.\n");
        return -1;

    }
    fprintf(stdout, "\n");

    // modify the rdr
    ptr1->RdrType = SAHPI_CTRL_RDR;
    ptr1->RdrTypeUnion.CtrlRec.Num = 1;
    ptr1->RdrTypeUnion.CtrlRec.OutputType = SAHPI_CTRL_LCD_DISPLAY;
    ptr1->RdrTypeUnion.CtrlRec.Type = SAHPI_CTRL_TYPE_TEXT;
    ptr1->RdrTypeUnion.CtrlRec.TypeUnion.Text.MaxChars = 80;
    ptr1->RdrTypeUnion.CtrlRec.TypeUnion.Text.MaxLines = 1;
    ptr1->RdrTypeUnion.CtrlRec.TypeUnion.Text.Language = SAHPI_LANG_ENGLISH;
    ptr1->RdrTypeUnion.CtrlRec.TypeUnion.Text.DataType = SAHPI_TL_TYPE_TEXT;
    ptr1->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Line = 1;
    ptr1->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.Language = SAHPI_LANG_ENGLISH;
    ptr1->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.DataType = SAHPI_TL_TYPE_TEXT;
    strcpy((char *)ptr1->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.Data, "My string");
    ptr1->RdrTypeUnion.CtrlRec.TypeUnion.Text.Default.Text.DataLength = 9;
    ptr1->RdrTypeUnion.CtrlRec.DefaultMode.Mode = SAHPI_CTRL_MODE_AUTO;
    ptr1->RdrTypeUnion.CtrlRec.DefaultMode.ReadOnly = true;
    ptr1->RdrTypeUnion.CtrlRec.WriteOnly = false;
    ptr1->RdrTypeUnion.CtrlRec.Oem = 9;

    // print the contents of the first rdr
    fprintf(stdout, "\nSaHpiRdr\n");
    if (ptr1->fprint(stdout, 3)) {
        printf("Error: Unable to print the buffer.\n");
        return -1;

    }
    fprintf(stdout, "\n");

    printf("Success!\n");
    return 0;
}

