/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 27 Mar 2009  0.7  Cleanup ret and errno variable names and dprintf() outputs;
 *                   Make VIDIOC_S_STD tests independent from VIDIOC_G_STD
 *  9 Feb 2009  0.6  Modify test cases to support drivers without any inputs;
 *                   cleanup debug printouts
 * 30 Jan 2009  0.5  valid_v4l2_std_id() moved to v4l2_validator.c
 * 18 Jan 2009  0.4  Typo corrected
 * 23 Dec 2008  0.3  Debug messages added
 * 22 Dec 2008  0.2  Test case with NULL parameter added
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

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"
#include "v4l2_validator.h"

#include "test_VIDIOC_STD.h"

void test_VIDIOC_G_STD() {
	int ret_get, errno_get;
	v4l2_std_id std_id;

	memset(&std_id, 0xff, sizeof(std_id));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_STD, &std_id);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_STD, ret_get=%i, errno_get=%i, std_id=0x%llX\n",
		__FILE__, __LINE__, ret_get, errno_get, std_id);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT(valid_v4l2_std_id(std_id));
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}

}

static int do_set_video_standard(int f, v4l2_std_id id) {
	int ret_set, errno_set;
	int ret_get, errno_get;
	v4l2_std_id std_id;

	std_id = id;
	ret_set = ioctl(f, VIDIOC_S_STD, &std_id);
	errno_set = errno;

	dprintf("\t%s:%u: ret_set=%i, errno_set=%i, std_id=0x%llX, id=0x%llX\n",
		__FILE__, __LINE__, ret_set, errno_set, std_id, id);

	if (ret_set == 0) {
		CU_ASSERT_EQUAL(ret_set, 0);
		memset(&std_id, 0xff, sizeof(std_id));
		ret_get = ioctl(f, VIDIOC_G_STD, &std_id);
		errno_get = errno;

		CU_ASSERT_EQUAL(ret_get, 0);
		if (ret_get == 0) {
			CU_ASSERT( (id & std_id) == id);

			if (std_id != id) {
				dprintf("\t%s:%u: ret_get=%i, errno_get=%i, std_id=0x%llX, id=0x%llX\n",
					__FILE__, __LINE__, ret_get, errno_get, std_id, id);
			}

		}
	} else {
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}

	return ret_set;
}

void test_VIDIOC_S_STD() {
	int ret_get, errno_get;
	int ret_set, errno_set;
	v4l2_std_id std_id_orig;
	int f;

	f = get_video_fd();

	memset(&std_id_orig, 0xff, sizeof(std_id_orig));
	ret_get = ioctl(f, VIDIOC_G_STD, &std_id_orig);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_STD: ret_get=%i, errno_get=%i, std_id_orig=0x%llX\n",
		__FILE__, __LINE__, ret_get, errno_get, std_id_orig);

	ret_set = do_set_video_standard(f, V4L2_STD_PAL_B);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_B1);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_G);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_H);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_I);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_D);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_D1);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_K);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_M);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_N);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_Nc);
	ret_set = do_set_video_standard(f, V4L2_STD_PAL_60);
	ret_set = do_set_video_standard(f, V4L2_STD_NTSC_M);
	ret_set = do_set_video_standard(f, V4L2_STD_NTSC_M_JP);
	ret_set = do_set_video_standard(f, V4L2_STD_NTSC_443);
	ret_set = do_set_video_standard(f, V4L2_STD_NTSC_M_KR);
	ret_set = do_set_video_standard(f, V4L2_STD_SECAM_B);
	ret_set = do_set_video_standard(f, V4L2_STD_SECAM_D);
	ret_set = do_set_video_standard(f, V4L2_STD_SECAM_G);
	ret_set = do_set_video_standard(f, V4L2_STD_SECAM_H);
	ret_set = do_set_video_standard(f, V4L2_STD_SECAM_K);
	ret_set = do_set_video_standard(f, V4L2_STD_SECAM_K1);
	ret_set = do_set_video_standard(f, V4L2_STD_SECAM_L);
	ret_set = do_set_video_standard(f, V4L2_STD_SECAM_LC);
	ret_set = do_set_video_standard(f, V4L2_STD_ATSC_8_VSB);
	ret_set = do_set_video_standard(f, V4L2_STD_ATSC_16_VSB);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		/* Setting the original std_id should not fail */
		ret_set = do_set_video_standard(f, std_id_orig);
		errno_set = errno;

		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}

}

void test_VIDIOC_S_STD_from_enum() {
	int ret_get, errno_get;
	int ret_enum, errno_enum;
	int ret_set, errno_set;
	v4l2_std_id std_id_orig;
	struct v4l2_standard std;
	__u32 i;
	int f;

	f = get_video_fd();

	memset(&std_id_orig, 0xff, sizeof(std_id_orig));
	ret_get = ioctl(f, VIDIOC_G_STD, &std_id_orig);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_STD: ret_get=%i, errno_get=%i, std_id_orig=0x%llX\n",
		__FILE__, __LINE__, ret_get, errno_get, std_id_orig);

	/* Try to continue even if VIDIOC_G_STD returned error */
	i = 0;
	do {
		memset(&std, 0xff, sizeof(std));
		std.index = i;
		ret_enum = ioctl(f, VIDIOC_ENUMSTD, &std);
		errno_enum = errno;

		dprintf("\t%s:%u: VIDIOC_ENUMSTD: i=%u, ret_enum=%i, errno_enum=%i, std.id=0x%llX\n",
			__FILE__, __LINE__, i, ret_enum, errno_enum, std.id);

		if (ret_enum == 0) {
			ret_set = do_set_video_standard(f, std.id);
			CU_ASSERT_EQUAL(ret_set, 0);
		}
		i++;
	} while (ret_enum == 0 && i != 0);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		/* Setting the original std_id should not fail */
		ret_set = do_set_video_standard(f, std_id_orig);
		errno_set = errno;
		dprintf("\t%s:%u: VIDIOC_S_STD: ret_set=%i (expected %i), errno=%i\n",
			__FILE__, __LINE__, ret_set, 0, errno);
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}

}


void test_VIDIOC_S_STD_invalid_standard() {
	int ret_get, errno_get;
	int ret_set, errno_set;
	v4l2_std_id std_id_orig;
	v4l2_std_id std_id;
	int f;

	f = get_video_fd();

	memset(&std_id_orig, 0xff, sizeof(std_id_orig));
	ret_get = ioctl(f, VIDIOC_G_STD, &std_id_orig);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_STD: ret_get=%i, errno_get=%i, std_id_orig=0x%llX\n",
		__FILE__, __LINE__, ret_get, errno_get, std_id_orig);

	/* Try to continue even if VIDIOC_G_STD retunred with error */
	std_id = 1;
	while (std_id != 0) {
		if (!valid_v4l2_std_id(std_id)) {
			ret_set = do_set_video_standard(f, std_id);
			errno_set = errno;

			CU_ASSERT_EQUAL(ret_set, -1);
			CU_ASSERT_EQUAL(errno_set, EINVAL);
			dprintf("\t%s:%u: VIDIOC_S_STD: ret_set=%i, errno_set=%i\n",
				__FILE__, __LINE__, ret_set, errno_set);
		}
		std_id = std_id<<1;
	}

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		/* Setting the original std_id should not fail */
		ret_set = do_set_video_standard(f, std_id_orig);
		errno_set = errno;

		dprintf("\t%s:%u: VIDIOC_S_STD: ret_set=%i (expected 0), errno=%i\n",
			__FILE__, __LINE__, ret_set, errno_set);
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}
}

void test_VIDIOC_G_STD_NULL() {
	int ret_get, errno_get;
	int ret_null, errno_null;
	v4l2_std_id std_id;

	memset(&std_id, 0, sizeof(std_id));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_STD, &std_id);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_STD, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_STD, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_S_STD: ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}

void test_VIDIOC_S_STD_NULL() {
	int ret_null, errno_null;

	/* TODO: check whether VIDIOC_S_STD is supported at all or not */

	ret_null = ioctl(get_video_fd(), VIDIOC_S_STD, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_S_STD: ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	CU_ASSERT_EQUAL(ret_null, -1);
	CU_ASSERT_EQUAL(errno_null, EFAULT);

}


/* TODO: VIDIOC_S_STD while STREAM_ON */
