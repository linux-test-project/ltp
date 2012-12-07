/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  5 Apr 2009  0.3  Test case for NULL parameter reworked
 * 28 Mar 2009  0.2  Clean up ret and errno variable names and dprintf() output
 * 30 Jan 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include <linux/videodev2.h>
#include <linux/errno.h>

#include <CUnit/CUnit.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"
#include "v4l2_validator.h"

#include "test_VIDIOC_QUERYSTD.h"

void test_VIDIOC_QUERYSTD()
{
	int ret_query, errno_query;
	v4l2_std_id id;

	memset(&id, 0xff, sizeof(id));

	ret_query = ioctl(get_video_fd(), VIDIOC_QUERYSTD, &id);
	errno_query = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_QUERYSTD, ret_query=%i, errno_query=%i, id=0x%llx\n",
	     __FILE__, __LINE__, ret_query, errno_query, id);

	if (ret_query == 0) {
		CU_ASSERT_EQUAL(ret_query, 0);
		CU_ASSERT(id != 0);
		CU_ASSERT(valid_v4l2_std_id(id));

	} else {
		/* if this ioctl is not supported, then errno shall be EINVAL */
		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);
	}

}

void test_VIDIOC_QUERYSTD_NULL()
{
	int ret_query, errno_query;
	int ret_null, errno_null;
	v4l2_std_id id;

	memset(&id, 0, sizeof(id));
	ret_query = ioctl(get_video_fd(), VIDIOC_QUERYSTD, &id);
	errno_query = errno;

	dprintf("\t%s:%u: VIDIOC_QUERYSTD: ret_query=%i, errno_query=%i\n",
		__FILE__, __LINE__, ret_query, errno_query);

	ret_null = ioctl(get_video_fd(), VIDIOC_QUERYSTD, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_G_TUNER: ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	/* check if VIDIOC_QUERYSTD is supported at all or not */
	if (ret_query == 0) {
		/* VIDIOC_QUERYSTD is supported, the parameter should be checked */
		CU_ASSERT_EQUAL(ret_query, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		/* VIDIOC_QUERYSTD not supported at all, the parameter should not be evaluated */
		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);

	}
}

/* TODO: check for different input settings */
