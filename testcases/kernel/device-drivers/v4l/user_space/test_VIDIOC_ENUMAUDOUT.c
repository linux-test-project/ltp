/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.4  Added string content validation
 * 18 Apr 2009  0.3  More strict check for strings
 * 25 Mar 2009  0.2  Typos corrected, cleaned up dprintf() outputs;
 *                   cleaned up ret and errno variable names
 *  1 Jan 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 *
 * Note: in ioctl VIDIOC_ENUMAUDOUT reference (doc/spec/r8304.htm)
 * the description text mentions the VIDIOC_G_AUDOUT should
 * be called. This is a typo in the V4L2 specification (revision 0.24).
 * To enumerate the audio outputs the VIDIOC_ENUMAUDOUT should be
 * called.
 *
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
#include "v4l2_validator.h"

#include "test_VIDIOC_ENUMAUDOUT.h"

void test_VIDIOC_ENUMAUDOUT()
{
	int ret_enum, errno_enum;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;
	__u32 i;

	i = 0;
	do {
		memset(&audioout, 0xff, sizeof(audioout));
		audioout.index = i;
		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, &audioout);
		errno_enum = errno;

		dprintf("\tVIDIOC_ENUMAUDOUT, ret_enum=%i, errno_enum=%i\n",
			ret_enum, errno_enum);

		if (ret_enum == 0) {
			CU_ASSERT_EQUAL(ret_enum, 0);
			CU_ASSERT_EQUAL(audioout.index, i);

			CU_ASSERT(0 < strlen((char *)audioout.name));
			CU_ASSERT(valid_string
				  ((char *)audioout.name,
				   sizeof(audioout.name)));

			//CU_ASSERT_EQUAL(audioout.capability, ?);
			//CU_ASSERT_EQUAL(audioout.mode, ?);
			CU_ASSERT_EQUAL(audioout.reserved[0], 0);
			CU_ASSERT_EQUAL(audioout.reserved[1], 0);

			/* Check if the unused bytes of the name string are
			 * also filled with zeros. Also check if there is any
			 * padding byte between any two fields then this
			 * padding byte is also filled with zeros.
			 */
			memset(&audioout2, 0, sizeof(audioout2));
			audioout2.index = audioout.index;
			strncpy((char *)audioout2.name, (char *)audioout.name,
				sizeof(audioout2.name));
			audioout2.capability = audioout.capability;
			audioout2.mode = audioout.mode;
			CU_ASSERT_EQUAL(memcmp
					(&audioout, &audioout2,
					 sizeof(audioout)), 0);

			dprintf("\taudioout = {.index=%u, .name=\"%s\", "
				".capability=0x%X, .mode=0x%X, "
				".reserved[]={ 0x%X, 0x%X } }\n",
				audioout.index,
				audioout.name,
				audioout.capability,
				audioout.mode,
				audioout.reserved[0], audioout.reserved[1]
			    );

		} else {
			CU_ASSERT_EQUAL(ret_enum, -1);
			CU_ASSERT_EQUAL(errno_enum, EINVAL);

			/* check whether the structure is untouched */
			memset(&audioout2, 0xff, sizeof(audioout2));
			audioout2.index = i;
			CU_ASSERT_EQUAL(memcmp
					(&audioout, &audioout2,
					 sizeof(audioout)), 0);

		}
		i++;
	} while (ret_enum == 0);

}

void test_VIDIOC_ENUMAUDOUT_S32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;

	memset(&audioout, 0xff, sizeof(audioout));
	audioout.index = (__u32) S32_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, &audioout);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original audioout struct is untouched */
	memset(&audioout2, 0xff, sizeof(audioout2));
	audioout2.index = (__u32) S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);
}

void test_VIDIOC_ENUMAUDOUT_S32_MAX_1()
{
	int ret_enum, errno_enum;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;

	memset(&audioout, 0xff, sizeof(audioout));
	audioout.index = ((__u32) S32_MAX) + 1;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, &audioout);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original audioout struct is untouched */
	memset(&audioout2, 0xff, sizeof(audioout2));
	audioout2.index = ((__u32) S32_MAX) + 1;
	CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);
}

void test_VIDIOC_ENUMAUDOUT_U32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;

	memset(&audioout, 0xff, sizeof(audioout));
	audioout.index = U32_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, &audioout);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original audioout struct is untouched */
	memset(&audioout2, 0xff, sizeof(audioout2));
	audioout2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);
}

void test_VIDIOC_ENUMAUDOUT_NULL()
{
	int ret_enum, errno_enum;
	struct v4l2_audioout audioout;
	int ret_null, errno_null;

	memset(&audioout, 0, sizeof(audioout));
	audioout.index = 0;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, &audioout);
	errno_enum = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMAUDOUT, ret_enum=%i, errno_enum=%i\n",
		__FILE__, __LINE__, ret_enum, errno_enum);

	ret_null = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMAUDOUT, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_enum == 0) {
		CU_ASSERT_EQUAL(ret_enum, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_enum, -1);
		CU_ASSERT_EQUAL(errno_enum, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}
