/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * This test case will cover all the conversion specifiers that are supported 
 * in strftime().
 */


#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <langinfo.h>
#include <time.h> 
#include "posixtest.h"


int main () {

    /* current time */
    time_t t = time(NULL);
    struct tm* local_t = localtime(&t);
    char text[256];
    int result;

    setlocale(LC_TIME, "");
    strftime(text, 256, nl_langinfo (D_T_FMT), local_t);
    printf("STRING IS:   %s\n\n", text);


    /* use format controls to print the various date/time components. */

    result = strftime(text, 256, "%a", local_t);
    printf("a   Bytes %i           %s	", result, text);
    if ( result != 3 ) {
	    puts("Test Failed: \%a doesn't equal at least 3 bytes");
	    return PTS_FAIL;
    } else {
	    puts("PASS");
    }

    result = strftime(text, 256, "%A", local_t);
    printf("A   Bytes %i           %s	", result, text);
    if ( result <= 5 ) {
	    puts("Test Failed: \%A doesn't equal to 6 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256 , "%b", local_t);
    printf("b   Bytes %i           %s	", result, text);
    if ( result != 3 ) {
	    puts("Test Failed: \%b doesn't equal to 3 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%B", local_t);
    printf("B   Bytes %i           %s	", result, text);
    if ( result < 3 ) {
	    puts("Test Failed: \%B is less than3 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

/*  This section has been commented for known bug in gcc:
 *
 *  result = strftime(text, 256, "%c", local_t);
 *  printf("c   Bytes %i          %s	", result, text);
 *  if ( result <= 30 ) {
 *    puts("Test Failed: \%c doesn't equal at least 30 bytes");
 *   } else {
 *    puts ("PASS");
 *   }
 */

    result = strftime(text, 256, "%C", local_t);
    printf("C   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%C doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%d", local_t);
    printf("d   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%d doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

/*  This section has been commented for known bug in gcc:
 *
 *  result = strftime(text, 256, "%D", local_t);
 *  printf("D   Bytes %i           %s	", result, text);
 *  if ( result != 8 ) {
 *    puts("Test Failed: \%D doesn't equal at least 2 bytes");
 *    return PTS_FAIL;
 *   } else {
 *    puts ("PASS");
 *   }
 */


    result = strftime(text, 256, "%e", local_t);
    printf("e   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%e doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%F", local_t);
    printf("F   Bytes %i           %s	", result, text);
    if ( result != 10 ) {
	    puts("Test Failed: \%F doesn't equal at least 10 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%H", local_t);
    printf("H   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%H doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

/*  This section has been commented for known bug in gcc:
 *
 *   result = strftime(text, 256, "%g", local_t);
 *   printf("g   Bytes %i           %s		", result, text);
 *   if ( result != 2 ) {
*	    puts("Test Failed: \%g doesn't equal at least 2 bytes");
*	    return PTS_FAIL;
*    } else {
*	    puts ("PASS");
*    }
*/

    result = strftime(text, 256, "%G", local_t);
    printf("G   Bytes %i           %s	", result, text);
    if ( result != 4 ) {
	    puts("Test Failed: \%G doesn't equal at least 4 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%h", local_t);
    printf("h   Bytes %i           %s	", result, text);
    if ( result != 3 ) {
	    puts("Test Failed: \%h doesn't equal at least 3 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }



    result = strftime(text, 256, "%I", local_t);
    printf("I   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%I doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%j", local_t);
    printf("j   Bytes %i           %s	", result, text);
    if ( result != 3 ) {
	    puts("Test Failed: \%j doesn't equal at least 3 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%m", local_t);
    printf("m   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%m doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%M", local_t);
    printf("M   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%M doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%n", local_t);
    printf("n   Bytes %i           %s	", result, text);
    if ( result != 1 ) {
	    puts("Test Failed: \%n doesn't equal at least 1 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }


    result = strftime(text, 256, "%p", local_t);
    printf("p   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%p doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%r", local_t);
    printf("r   Bytes %i           %s	", result, text);
    if ( result != 11 ) {
	    puts("Test Failed: \%r doesn't equal at least 11 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }


    result = strftime(text, 256, "%R", local_t);
    printf("R   Bytes %i           %s	", result, text);
    if ( result != 5 ) {
	    puts("Test Failed: \%R doesn't equal at least 5 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }


    result = strftime(text, 256, "%S", local_t);
    printf("S   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%S doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%t", local_t);
    printf("t   Bytes %i           %s	", result, text);
    if ( result != 1 ) {
	    puts("Test Failed: \%t doesn't equal at least 1 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }


    result = strftime(text, 256, "%T", local_t);
    printf("T   Bytes %i           %s	", result, text);
    if ( result != 8 ) {
	    puts("Test Failed: \%T doesn't equal at least 8 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }


    result = strftime(text, 256, "%u", local_t);
    printf("u   Bytes %i           %s	", result, text);
    if ( result != 1 ) {
	    puts("Test Failed: \%u doesn't equal at least 1 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }


    result = strftime(text, 256, "%U", local_t);
    printf("U   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%U doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%V", local_t);
    printf("V   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%V doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }


    result = strftime(text, 256, "%w", local_t);
    printf("w   Bytes %i           %s	", result, text);
    if ( result != 1 ) {
	    puts("Test Failed: \%w doesn't equal at least 1 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%W", local_t);
    printf("W   Bytes %i           %s	", result, text);
    if ( result != 2 ) {
	    puts("Test Failed: \%W doesn't equal at least 2 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

/*  This section has been commented for known bug in gcc:
 *
 *  result = strftime(text, 256, "%x", local_t);
 *  printf("x   Bytes %i           %s	", result, text);
 *  if ( result != 10 ) {
 *    puts("Test Failed: \%x doesn't equal at least 10 bytes");
 *    return PTS_FAIL;
 *   } else {
 *    puts ("PASS");
 *   }
 */

    result = strftime(text, 256, "%X", local_t);
    printf("X   Bytes %i           %s	", result, text);
    if ( result < 8 ) {
	    puts("Test Failed: \%X doesn't equal at least 8 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

/*  This section has been commented for known bug in gcc:
 *
 *  result = strftime(text, 256, "%y", local_t);
 *  printf("y   Bytes %i           %s	", result, text);
 *  if ( result != 2 ) {
 *    puts("Test Failed: \%y doesn't equal at least 2 bytes");
 *    return PTS_FAIL;
 *   } else {
 *    puts ("PASS");
 *   }
 */

    result = strftime(text, 256, "%Y", local_t);
    printf("Y   Bytes %i           %s	", result, text);
    if ( result != 4 ) {
	    puts("Test Failed: \%Y doesn't equal at least 4 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }

    result = strftime(text, 256, "%z", local_t);
    printf("z   Bytes %i           %s	", result, text);
    if ( result != 5 ) {
	    puts("Test Failed: \%z doesn't equal at least 5 bytes");
	    return PTS_FAIL;
    } else {
	    puts ("PASS");
    }


    //result = strftime(text, 256, "%Z", local_t);
  //printf("Z   Bytes %i           %s	", result, text);
  //if ( result != 3 ) {
//	    puts("Test Failed: \%Z doesn't equal at least 3 bytes");
 //		return PTS_FAIL;
   // 	} else {
//		    puts ("PASS");
  //  	}
    printf("\n");

return PTS_PASS;
}
