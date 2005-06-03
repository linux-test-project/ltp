  /*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

   @pt:CX
  Test that tzname is declared.
  */

#include <time.h>
#include <string.h>

int dummyfcn (void)
{
	char *dummy[10];

	strcpy(*dummy,*tzname);
	return 0;
}
