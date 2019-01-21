/*
  Test for the existence and valid prototype
  of the mq_timedreceive function as specified on
  line 9692 of the Base Definitions document
*/

#include <mqueue.h>
#include <time.h>
#include "posixtest.h"

void test_mq_timedreceive_prototype(void)
{
	mqd_t mqdes;
	struct timespec abstime;
	size_t msg_len;
	ssize_t size;
	char *msgp;
	unsigned msg_prio;

	mqdes = 0;
	msg_len = 0;
	msgp = NULL;

	size = mq_timedreceive(mqdes, msgp, msg_len, &msg_prio, &abstime);
	(void)size;
}
