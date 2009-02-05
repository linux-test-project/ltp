/*
 * v4l-test: Test environment for Video For Linux Two API
 *
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

int valid_priority(enum v4l2_priority priority) {
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
	int ret;
	enum v4l2_priority new_priority;

	dprintf("\t%s:%u: set priority to %i\n", __FILE__, __LINE__, priority);
	ret = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &priority);

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY, ret=%i\n", __FILE__, __LINE__, ret);

	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		memset(&new_priority, 0xff, sizeof(new_priority));
		ret = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &new_priority);
		CU_ASSERT_EQUAL(ret, 0);
		if (ret == 0) {
			CU_ASSERT_EQUAL(new_priority, priority);
		}
	}
}

static void do_set_invalid_priority(enum v4l2_priority orig_priority,
				    enum v4l2_priority priority)
{
	int ret;
	enum v4l2_priority new_priority;

	dprintf("\t%s:%u: try to set priority to %i\n", __FILE__, __LINE__, priority);
	ret = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &priority);

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY, ret=%i\n", __FILE__, __LINE__, ret);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);
	if (ret == -1 && errno == EINVAL) {
		memset(&new_priority, 0xff, sizeof(new_priority));
		ret = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &new_priority);
		CU_ASSERT_EQUAL(ret, 0);
		if (ret == 0) {
			CU_ASSERT_EQUAL(new_priority, orig_priority);
		}
	}
}

void test_VIDIOC_G_PRIORITY()
{
	int ret;
	enum v4l2_priority orig_priority;

	memset(&orig_priority, 0xff, sizeof(orig_priority));
	ret = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &orig_priority);

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY, ret=%i\n", __FILE__, __LINE__, ret);

	if (ret == 0) {
		CU_ASSERT_EQUAL(ret, 0);
		CU_ASSERT(valid_priority(orig_priority));

		dprintf("\torig_priority = %u\n", orig_priority);
	} else {
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

	}

}

void test_VIDIOC_G_PRIORITY_NULL()
{
	int ret1;
	int errno1;
	int ret2;
	int errno2;
	enum v4l2_priority priority;

	memset(&priority, 0xff, sizeof(priority));
	ret1 = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &priority);
	errno1 = errno;

	ret2 = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, NULL);
	errno2 = errno;

	/* check if VIDIOC_G_PRIORITY is supported at all or not */
	if (ret1 == -1 && errno1 == EINVAL) {
		/* VIDIOC_G_PRIORITY not supported at all, the parameter should not be evaluated */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno2=%d (expected %d)\n", __FILE__, __LINE__, errno2, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EINVAL);

	} else {
		/* VIDIOC_G_PRIORITY is supported, the parameter should be checked */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno2=%d (expected %d)\n", __FILE__, __LINE__, errno2, EFAULT);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EFAULT);
	}
}

void test_VIDIOC_S_PRIORITY()
{
	int ret;
	enum v4l2_priority orig_priority;

	memset(&orig_priority, 0xff, sizeof(orig_priority));
	ret = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &orig_priority);

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY, ret=%i\n", __FILE__, __LINE__, ret);

	if (ret == 0) {
		CU_ASSERT_EQUAL(ret, 0);
		CU_ASSERT(valid_priority(orig_priority));

		dprintf("\torig_priority = %u\n", orig_priority);

		do_set_priority(V4L2_PRIORITY_UNSET);
		do_set_priority(V4L2_PRIORITY_BACKGROUND);
		do_set_priority(V4L2_PRIORITY_INTERACTIVE);
		do_set_priority(V4L2_PRIORITY_RECORD);

		CU_ASSERT_EQUAL(V4L2_PRIORITY_DEFAULT, V4L2_PRIORITY_INTERACTIVE);

		do_set_priority(orig_priority);

	} else {
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

	}
}

void test_VIDIOC_S_PRIORITY_invalid()
{
	int ret;
	enum v4l2_priority orig_priority;

	memset(&orig_priority, 0xff, sizeof(orig_priority));
	ret = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &orig_priority);

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY, ret=%i\n", __FILE__, __LINE__, ret);

	if (ret == 0) {
		CU_ASSERT_EQUAL(ret, 0);
		CU_ASSERT(valid_priority(orig_priority));

		dprintf("\torig_priority = %u\n", orig_priority);

		do_set_invalid_priority(orig_priority, 4);
		do_set_invalid_priority(orig_priority, S32_MAX);
		do_set_invalid_priority(orig_priority, ((__u32)S32_MAX)+1);
		do_set_invalid_priority(orig_priority, U32_MAX);

		do_set_priority(orig_priority);

	} else {
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

	}
}

void test_VIDIOC_S_PRIORITY_NULL()
{
	int ret;
	int ret1;
	int errno1;
	int ret2;
	int errno2;
	enum v4l2_priority orig_priority;
	enum v4l2_priority priority;

	memset(&orig_priority, 0xff, sizeof(orig_priority));
	ret = ioctl(get_video_fd(), VIDIOC_G_PRIORITY, &orig_priority);

	dprintf("\t%s:%u: VIDIOC_G_PRIORITY, ret=%i\n", __FILE__, __LINE__, ret);

	if (ret == 0) {
		priority = orig_priority;
		ret1 = ioctl(get_video_fd(), VIDIOC_S_PRIORITY, &priority);
		errno1 = errno;

		ret2 = ioctl(get_video_fd(), VIDIOC_S_PRIORITY, NULL);
		errno2 = errno;

		/* check if VIDIOC_G_PRIORITY is supported at all or not */
		if (ret1 == -1 && errno1 == EINVAL) {
			/* VIDIOC_G_PRIORITY not supported at all, the parameter should not be evaluated */
			dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
			dprintf("\t%s:%u: errno2=%d (expected %d)\n", __FILE__, __LINE__, errno2, EINVAL);
			CU_ASSERT_EQUAL(ret2, -1);
			CU_ASSERT_EQUAL(errno2, EINVAL);

		} else {
			/* VIDIOC_G_PRIORITY is supported, the parameter should be checked */
			dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
			dprintf("\t%s:%u: errno2=%d (expected %d)\n", __FILE__, __LINE__, errno2, EFAULT);
			CU_ASSERT_EQUAL(ret2, -1);
			CU_ASSERT_EQUAL(errno2, EFAULT);
		}
	} else {

		/* VIDIOC_G_PRIORITY not supported, so shall be VIDIOC_S_PRIORITY */
		ret2 = ioctl(get_video_fd(), VIDIOC_S_PRIORITY, NULL);
		errno2 = errno;

		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno2=%d (expected %d)\n", __FILE__, __LINE__, errno2, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EINVAL);

	}

}
