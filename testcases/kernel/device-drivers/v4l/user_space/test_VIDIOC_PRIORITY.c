/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  4 Apr 2009  0.3  Test case for NULL parameter reworked
 * 27 Mar 2009  0.2  Correct VIDIOC_S_PRIORITY test cases
 *                   Clean up ret and errno variable names and dprintf() output
 *  2 Feb 2009  0.1  First release
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

#include "test_VIDIOC_PRIORITY.h"

int valid_priority(enum v4l2_priority priority)
{
	int valid = 0;

	CU_ASSERT_EQUAL(V4L2_PRIORITY_DEFAULT, V4L2_PRIORITY_INTERACTIVE);

	switch (priority) {
	case V4L2_PRIORITY_UNSET:
	case V4L2_PRIORITY_BACKGROUND:
	case V4L2_PRIORITY_INTERACTIVE:
	case V4L2_PRIORITY_RECORD:
		valid = 1;
		break;
	default:
		valid = 0;
	}
	return valid;
}

static void do_set_priority(enum v4l2_priority priority)
{
	int ret_set, errno_set;
	int ret_get, errno_get;
	enum v4l2_priority new_priority;

	dprintf("\t%s:%u: set priority to %i\n", __FILE__, __LINE__, priority);
	ret_set = ioctl(get_video_fd(), VIDIOC_S_PRIORITY, &priority);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_S_PRIORITY, ret_set=%i, errno_set=%i\n",
		__FILE__, __LINE__, ret_set, errno_set);

	CU_ASSERT_EQUAL(ret_set, 0);
	if (ret_set == 0) {
		memset(&new_priority, 0xff, sizeof(new_priority));
		ret_get =
		    ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &new_priority);
		errno_get = errno;

		CU_ASSERT_EQUAL(ret_get, 0);
		if (ret_get == 0) {
			CU_ASSERT_EQUAL(new_priority, priority);
		}
	}
}

static void do_set_invalid_priority(enum v4l2_priority orig_priority,
				    enum v4l2_priority priority)
{
	int ret_set, errno_set;
	int ret_get, errno_get;
	enum v4l2_priority new_priority;

	dprintf("\t%s:%u: try to set priority to %i\n", __FILE__, __LINE__,
		priority);
	ret_set = ioctl(get_video_fd(), VIDIOC_S_PRIORITY, &priority);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_S_PRIORITY, ret_set=%i, errno_set=%i\n",
		__FILE__, __LINE__, ret_set, errno_set);

	CU_ASSERT_EQUAL(ret_set, -1);
	CU_ASSERT_EQUAL(errno_set, EINVAL);
	if (ret_set == -1 && errno_set == EINVAL) {
		memset(&new_priority, 0xff, sizeof(new_priority));
		ret_get =
		    ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &new_priority);
		errno_get = errno;

		CU_ASSERT_EQUAL(ret_get, 0);
		if (ret_get == 0) {
			CU_ASSERT_EQUAL(new_priority, orig_priority);
		}
	}
}

void test_VIDIOC_G_PRIORITY()
{
	int ret_get, errno_get;
	enum v4l2_priority orig_priority;

	memset(&orig_priority, 0xff, sizeof(orig_priority));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &orig_priority);
	errno_get = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_PRIORITY, ret_get=%i, errno_get=%i, orig_priority=%i\n",
	     __FILE__, __LINE__, ret_get, errno_get, orig_priority);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT(valid_priority(orig_priority));

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

	}

}

void test_VIDIOC_G_PRIORITY_NULL()
{
	int ret_get, errno_get;
	int ret_null, errno_null;
	enum v4l2_priority priority;

	memset(&priority, 0xff, sizeof(priority));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &priority);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY: ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY: ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	/* check if VIDIOC_G_PRIORITY is supported at all or not */
	if (ret_get == -1 && errno_get == EINVAL) {
		/* VIDIOC_G_PRIORITY not supported at all, the parameter should not be evaluated */
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);

	} else {
		/* VIDIOC_G_PRIORITY is supported, the parameter should be checked */
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	}
}

void test_VIDIOC_S_PRIORITY()
{
	int ret_get, errno_get;
	enum v4l2_priority orig_priority;

	memset(&orig_priority, 0xff, sizeof(orig_priority));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &orig_priority);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT(valid_priority(orig_priority));

		dprintf("\torig_priority = %u\n", orig_priority);

		do_set_priority(V4L2_PRIORITY_UNSET);
		do_set_priority(V4L2_PRIORITY_BACKGROUND);
		do_set_priority(V4L2_PRIORITY_INTERACTIVE);
		do_set_priority(V4L2_PRIORITY_RECORD);

		CU_ASSERT_EQUAL(V4L2_PRIORITY_DEFAULT,
				V4L2_PRIORITY_INTERACTIVE);

		do_set_priority(orig_priority);

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

	}
}

void test_VIDIOC_S_PRIORITY_invalid()
{
	int ret_get, errno_get;
	enum v4l2_priority orig_priority;

	memset(&orig_priority, 0xff, sizeof(orig_priority));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &orig_priority);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT(valid_priority(orig_priority));

		dprintf("\torig_priority = %u\n", orig_priority);

		do_set_invalid_priority(orig_priority, 4);
		do_set_invalid_priority(orig_priority, S32_MAX);
		do_set_invalid_priority(orig_priority, ((__u32) S32_MAX) + 1);
		do_set_invalid_priority(orig_priority, U32_MAX);

		do_set_priority(orig_priority);

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

	}
}

void test_VIDIOC_S_PRIORITY_NULL()
{
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	int ret_null, errno_null;
	enum v4l2_priority priority_orig;
	enum v4l2_priority priority;

	memset(&priority_orig, 0, sizeof(priority_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &priority_orig);
	errno_orig = errno;

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY, ret_orig=%i, errno_orig=%i\n",
		__FILE__, __LINE__, ret_orig, errno_orig);

	if (ret_orig == 0) {
		priority = priority_orig;
	} else {
		priority = V4L2_PRIORITY_DEFAULT;
	}

	ret_set = ioctl(get_video_fd(), VIDIOC_S_PRIORITY, &priority);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_S_PRIORITY, ret_set=%d, errno_set=%i\n",
		__FILE__, __LINE__, ret_set, errno_set);

	ret_null = ioctl(get_video_fd(), VIDIOC_S_PRIORITY, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_S_PRIORITY, ret_null=%d, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	/* check if VIDIOC_S_PRIORITY is supported at all or not */
	if (ret_set == 0) {
		/* VIDIOC_S_PRIORITY is supported, the parameter should be checked */
		CU_ASSERT_EQUAL(ret_set, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		/* VIDIOC_S_PRIORITY not supported at all, the parameter should not be evaluated */
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}
