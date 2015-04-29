/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 *
 * Modified by the E or O indicate that an alternative format or
 * specification should be used rather than the one normally used by the
 * unmodified conversion specifier (1-1.c).
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <langinfo.h>
#include <time.h>
#include "posixtest.h"

int main(void)
{

	/* current time */
	time_t t = time(NULL);
	struct tm *local_t = localtime(&t);
	char text[256];
	int result;

	setlocale(LC_TIME, "");
	strftime(text, sizeof(text), nl_langinfo(D_T_FMT), local_t);
	printf("STRING IS:   %s\n\n", text);

/* use format controls to print the various date/time components. */

/*  This section has been commented for known bug in gcc:
 *  result = strftime(text, sizeof(text), "%Ec", local_t);
 *  printf("Ec   Bytes %i           %s	", result, text);
 *  if (result != 31) {
 *    puts("Test Failed: \%Ec doesn't equal at least 31 bytes");
 *    return PTS_FAIL;
 *   } else {
 *    puts("PASS");
 *   }
 */

	result = strftime(text, sizeof(text), "%EC", local_t);
	printf("EC   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%EC doesn't equal to 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

/*  This section has been commented for known bug in gcc:
 *  result = strftime(text, sizeof(text) , "%Ex", local_t);
 *  printf("Ex   Bytes %i           %s	", result, text);
 *  if (result != 10) {
 *    puts("Test Failed: \%Ex doesn't equal to 10 bytes");
 *    return PTS_FAIL;
 *   } else {
 *    puts ("PASS");
 *   }
 */

	result = strftime(text, sizeof(text), "%EX", local_t);
	printf("EX   Bytes %i           %s	", result, text);
	if (result <= 3) {
		puts("Test Failed: \%EX doesn't equal to 3 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

/*  This section has been commented for known bug in gcc:
 *  result = strftime(text, sizeof(text), "%Ey", local_t);
 *  printf("Ey   Bytes %i           %s	", result, text);
 *  if (result != 2) {
 *    puts("Test Failed: \%Ey doesn't equal at least 2 bytes");
 *    return PTS_FAIL;
 *   } else {
 *    puts ("PASS");
 *   }
 */

	result = strftime(text, sizeof(text), "%EY", local_t);
	printf("EY   Bytes %i           %s	", result, text);
	if (result != 4) {
		puts("Test Failed: \%EY doesn't equal at least 4 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%Od", local_t);
	printf("Od   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%Od doesn't equal at least 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%Oe", local_t);
	printf("Oe   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%Oe doesn't equal at least 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%OH", local_t);
	printf("OH   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%OH doesn't equal at least 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%OI", local_t);
	printf("OI   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%OI doesn't equal at least 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%Om", local_t);
	printf("Om   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%Om doesn't equal at least 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%OM", local_t);
	printf("OM   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%OM doesn't equal at least 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%OS", local_t);
	printf("OS   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%OS doesn't equal at least 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%Ou", local_t);
	printf("Ou   Bytes %i           %s	", result, text);
	if (result != 1) {
		puts("Test Failed: \%Ou doesn't equal at least 1 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%OU", local_t);
	printf("OU   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%OU doesn't equal at least 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%OV", local_t);
	printf("OV   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%OV doesn't equal at least 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%Ow", local_t);
	printf("Ow   Bytes %i           %s	", result, text);
	if (result != 1) {
		puts("Test Failed: \%Ow doesn't equal at least 1 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

	result = strftime(text, sizeof(text), "%OW", local_t);
	printf("OW   Bytes %i           %s	", result, text);
	if (result != 2) {
		puts("Test Failed: \%OW doesn't equal at least 2 bytes");
		return PTS_FAIL;
	} else {
		puts("PASS");
	}

/*  This section has been commented for known bug in gcc:
 *  result = strftime(text, sizeof(text), "%Oy", local_t);
 *  printf("Oy   Bytes %i           %s	", result, text);
 *  if (result != 2) {
 *    puts("Test Failed: \%Oy doesn't equal at least 2 bytes");
 *    return PTS_FAIL;
 *   } else {
 *    puts ("PASS");
 *   }
 */

	printf("\n");

	return PTS_PASS;
}
