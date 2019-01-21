/*
  Test for the existence and valid prototype
  of the mq_open function as specified on
  line 9687 of the Base Definitions document
*/

#include <fcntl.h>
#include <mqueue.h>
#include "posixtest.h"

void test_mq_open_prototype(void)
{
	mqd_t res;
	int oflag = O_RDONLY;
	const char *name = "bogus";

	res = mq_open(name, oflag);
	(void)res;
}
