/*
  Test for the existence and valid prototype
  of the mq_send function as specified on
  line 9689 of the Base Definitions document
*/

#include <mqueue.h>
#include "posixtest.h"

void test_mq_send_prototype(void)
{
	mqd_t mqdes;
	size_t msg_len;
	char *msgp;
	int err;
	unsigned msg_prio;

	err = mq_receive(mqdes, msgp, msg_len, &msg_prio);

}
