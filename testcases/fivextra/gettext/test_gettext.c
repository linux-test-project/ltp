/*##############################################################################
#
# File :	mytest.c
#
# Description:	This test is based on the plural-1-prg.c in gettext source.
#		It is used to test textdomain, bindtextdomain, ngettext and
#		gettext. 
#
# Author:	Andrew Pham <apham@us.ibm.com>
#
# History:	Mar 19 2003 - Created - RR
#
#
##############################################################################*/
#include <libintl.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

int main (argc, argv)
  int argc;
  char *argv[];
{
	int n = atoi (argv[1]);

	if (setlocale (LC_ALL, "") == NULL)
	{
		setlocale (LC_CTYPE, "");
		setlocale (LC_MESSAGES, "");
	}
	textdomain ("cake");
	bindtextdomain ("cake", "/tmp/gettext");
  
 	if ( n == -1)
		printf (gettext ("a piece of cake"));
	else
		printf (ngettext ("a piece of cake", "%d pieces of cake", n), n);

	printf ("\n");
	return 0;
}
