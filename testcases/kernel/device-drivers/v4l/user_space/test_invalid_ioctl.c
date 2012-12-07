/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 28 Mar 2009  0.2  Clean up ret and errno variable names
 * 18 Dec 2008  0.1  First release
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
#include <CUnit/Basic.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_invalid_ioctl.h"

/* invalid ioctls */

static void do_invalid_ioctl(int f, int request)
{
	int ret_invalid, errno_invalid;

	ret_invalid = ioctl(f, request, NULL);
	errno_invalid = errno;

	CU_ASSERT_EQUAL(ret_invalid, -1);
	CU_ASSERT_EQUAL(errno_invalid, EINVAL);
}

void test_invalid_ioctl_1()
{
	do_invalid_ioctl(get_video_fd(), _IO(0, 0));
}

void test_invalid_ioctl_2()
{
	do_invalid_ioctl(get_video_fd(), _IO(0xFF, 0xFF));
}

void test_invalid_ioctl_3()
{
	do_invalid_ioctl(get_video_fd(), _IO('v', 0xFF));
}

void test_invalid_ioctl_4()
{
	do_invalid_ioctl(get_video_fd(), _IO('V', 0xFF));
}
