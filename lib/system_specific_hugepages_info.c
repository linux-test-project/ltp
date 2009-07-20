/*
 *
 *   Copyright (c) International Business Machines  Corp., 2009
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *   DESCRIPTION
 *               get_no_of_hugepages() --> Return No. of hugepages for this systems
 *                                         from /proc/meminfo
 *               hugepages_size()      --> Return Hugepages Size for this system
 *                                         from /proc/meminfo
 */

#include <fcntl.h>
#include <sys/types.h>
#include <test.h>

#define BUFSIZE 512

int get_no_of_hugepages() {
 #ifdef __linux__
       FILE *f;
       char buf[BUFSIZ];

       f = popen("grep 'HugePages_Total' /proc/meminfo | cut -d ':' -f2 | tr -d ' \n'", "r");
       if (!f) {
               tst_resm(TBROK, "Could not get info about Total_Hugepages from /proc/meminfo");
               tst_exit();
       }
       if (!fgets(buf, 10, f)) {
               fclose(f);
               tst_resm(TBROK, "Could not read Total_Hugepages from /proc/meminfo");
               tst_exit();
       }
       pclose(f);
       return(atoi(buf));
 #else
        return -1;
 #endif
}


int hugepages_size() {
 #ifdef __linux__
       FILE *f;
       char buf[BUFSIZ];

       f = popen("grep 'Hugepagesize' /proc/meminfo | cut -d ':' -f2 | tr -d 'kB \n'", "r");
       if (!f) {
               tst_resm(TBROK, "Could not get info about HugePages_Size from /proc/meminfo");
               tst_exit();
       }
       if (!fgets(buf, 10, f)) {
               fclose(f);
               tst_resm(TBROK, "Could not read HugePages_Size from /proc/meminfo");
               tst_exit();
       }
       pclose(f);
       return(atoi(buf));
 #else
        return -1;
 #endif
}

