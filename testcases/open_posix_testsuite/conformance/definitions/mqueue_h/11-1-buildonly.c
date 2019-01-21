/*
Test for the existence and valid prototype
of the mq_unlink function as specified on
line 9696 of the Base Definitions document
*/

#include <mqueue.h>
#include "posixtest.h"

void test_mqueue_unlink_prototype()
{
	const char *name = "bogus";
	int err;

	err = mq_unlink(name);
	(void)err;
}
