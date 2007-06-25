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
#include "oSaHpiEntityPath.hpp"


int main(int argc, char *argv[]) {
    oSaHpiEntityPath *ent1, *ent2;

    // create the first entity path
    ent1 = new oSaHpiEntityPath;
    if (ent1 == NULL) {
        printf("Error: Unable to create a oSaHpiEntityPath.\n");
        return -1;
    }

    // set the initial entity path
    ent1->Entry[0].EntityType = SAHPI_ENT_SYSTEM_CHASSIS;
    ent1->Entry[0].EntityLocation = 1;

    // print the contents of the first entity path
    fprintf(stdout, "\nSaHpiEntityPath\n");
    if (ent1->fprint(stdout, 3)) {
        printf("Error: Unable to print the entity path.\n");
        return -1;

    }
    fprintf(stdout, "\n");

    // create the second entity path
    ent2 = new oSaHpiEntityPath;
    if (ent2 == NULL) {
        printf("Error: Unable to create a second oSaHpiEntityPath.\n");
        return -1;
    }

    // set the second initial entity path
    ent2->Entry[0].EntityType = SAHPI_ENT_PROCESSOR;
    ent2->Entry[0].EntityLocation = 1;

    // append the second entity pathe to the first
    ent1->append(ent2);

    // print the contents of the first appended entity path
    fprintf(stdout, "\nSaHpiEntityPath\n");
    if (ent1->fprint(stdout, 3)) {
        printf("Error: Unable to print the entity path.\n");
        return -1;

    }
    fprintf(stdout, "\n");

    // now compare the two entity paths
    if (ent1->compare(ent2) == true) {
        printf("Error: The two entity paths should not be equal.\n");
        return -1;
    }

    // copy the first to the second
    delete ent2;
    ent2 = new oSaHpiEntityPath(*ent1);

    // now compare the two entity paths
    if (ent1->compare(ent2) == false) {
        printf("Error: The two entity paths are not equal.\n");
        return -1;
    }

    printf("Success!\n");
    return 0;
}

