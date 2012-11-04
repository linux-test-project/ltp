/*
   Test for the existence and valid prototype
   of the mq_notify function as specified on
   line 9686 of the Base Definitions document
*/

#include <mqueue.h>
#include "posixtest.h"

void test_mq_notify_prototype(void)
{
	mqd_t mqdes;
	struct sigevent *notification;
	int err;

	err = mq_notify(mqdes, notification);

}
