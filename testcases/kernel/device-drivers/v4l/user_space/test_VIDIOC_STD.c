/*
 * v4l-test: Test environment for Video For Linux Two API
 *
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

#include "test_VIDIOC_STD.h"

int valid_v4l2_std_id(v4l2_std_id std_id) {
	int valid = 0;

	if ( (std_id & ~(V4L2_STD_PAL_B |
			 V4L2_STD_PAL_B1 |
			 V4L2_STD_PAL_G |
			 V4L2_STD_PAL_H |
			 V4L2_STD_PAL_I |
			 V4L2_STD_PAL_D |
			 V4L2_STD_PAL_D1 |
			 V4L2_STD_PAL_K |
			 V4L2_STD_PAL_M |
			 V4L2_STD_PAL_N |
			 V4L2_STD_PAL_Nc |
			 V4L2_STD_PAL_60 |
			 V4L2_STD_NTSC_M |
			 V4L2_STD_NTSC_M_JP |
			 V4L2_STD_NTSC_443 |
			 V4L2_STD_NTSC_M_KR |
			 V4L2_STD_SECAM_B |
			 V4L2_STD_SECAM_D |
			 V4L2_STD_SECAM_G |
			 V4L2_STD_SECAM_H |
			 V4L2_STD_SECAM_K |
			 V4L2_STD_SECAM_K1 |
			 V4L2_STD_SECAM_L |
			 V4L2_STD_SECAM_LC |
			 V4L2_STD_ATSC_8_VSB |
			 V4L2_STD_ATSC_16_VSB))
		== 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

void test_VIDIOC_G_STD() {
	int ret;
	v4l2_std_id std_id;

	memset(&std_id, 0xff, sizeof(std_id));
	ret = ioctl(get_video_fd(), VIDIOC_G_STD, &std_id);

	dprintf("VIDIOC_G_STD, ret=%i\n", ret);

	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		CU_ASSERT(valid_v4l2_std_id(std_id));

		dprintf("std_id=0x%llX\n", std_id);

	}

}

static int do_set_video_standard(int f, v4l2_std_id id) {
	int ret;
	int ret_set;
	v4l2_std_id std_id;

	std_id = id;
	ret_set = ioctl(f, VIDIOC_S_STD, &std_id);
	if (ret_set == 0) {
		CU_ASSERT_EQUAL(ret_set, 0);
		memset(&std_id, 0xff, sizeof(std_id));
		ret = ioctl(f, VIDIOC_G_STD, &std_id);
		CU_ASSERT_EQUAL(ret, 0);
		if (ret == 0) {
			CU_ASSERT( (id & std_id) == id);

			if (std_id != id) {
				dprintf("ret=%i, errno=%i, std_id=0x%llX, id=0x%llX\n", ret, errno, std_id, id);
			}

		}
	} else {
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

		if (ret_set != -1) {
			dprintf("ret_set=%i, errno=%i\n", ret_set, errno);
		}
	}

	return ret_set;
}

void test_VIDIOC_S_STD() {
	int ret;
	v4l2_std_id std_id_orig;
	int f;

	f = get_video_fd();

	memset(&std_id_orig, 0xff, sizeof(std_id_orig));
	ret = ioctl(f, VIDIOC_G_STD, &std_id_orig);
	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		ret = do_set_video_standard(f, V4L2_STD_PAL_B);
		ret = do_set_video_standard(f, V4L2_STD_PAL_B1);
		ret = do_set_video_standard(f, V4L2_STD_PAL_G);
		ret = do_set_video_standard(f, V4L2_STD_PAL_H);
		ret = do_set_video_standard(f, V4L2_STD_PAL_I);
		ret = do_set_video_standard(f, V4L2_STD_PAL_D);
		ret = do_set_video_standard(f, V4L2_STD_PAL_D1);
		ret = do_set_video_standard(f, V4L2_STD_PAL_K);
		ret = do_set_video_standard(f, V4L2_STD_PAL_M);
		ret = do_set_video_standard(f, V4L2_STD_PAL_N);
		ret = do_set_video_standard(f, V4L2_STD_PAL_Nc);
		ret = do_set_video_standard(f, V4L2_STD_PAL_60);
		ret = do_set_video_standard(f, V4L2_STD_NTSC_M);
		ret = do_set_video_standard(f, V4L2_STD_NTSC_M_JP);
		ret = do_set_video_standard(f, V4L2_STD_NTSC_443);
		ret = do_set_video_standard(f, V4L2_STD_NTSC_M_KR);
		ret = do_set_video_standard(f, V4L2_STD_SECAM_B);
		ret = do_set_video_standard(f, V4L2_STD_SECAM_D);
		ret = do_set_video_standard(f, V4L2_STD_SECAM_G);
		ret = do_set_video_standard(f, V4L2_STD_SECAM_H);
		ret = do_set_video_standard(f, V4L2_STD_SECAM_K);
		ret = do_set_video_standard(f, V4L2_STD_SECAM_K1);
		ret = do_set_video_standard(f, V4L2_STD_SECAM_L);
		ret = do_set_video_standard(f, V4L2_STD_SECAM_LC);
		ret = do_set_video_standard(f, V4L2_STD_ATSC_8_VSB);
		ret = do_set_video_standard(f, V4L2_STD_ATSC_16_VSB);

		/* Setting the original std_id should not fail */
		ret = do_set_video_standard(f, std_id_orig);
		CU_ASSERT_EQUAL(ret, 0);
	}

}

void test_VIDIOC_S_STD_from_enum() {
	int ret;
	int enum_ret;
	v4l2_std_id std_id_orig;
	struct v4l2_standard std;
	__u32 i;
	int f;

	f = get_video_fd();

	memset(&std_id_orig, 0xff, sizeof(std_id_orig));
	ret = ioctl(f, VIDIOC_G_STD, &std_id_orig);
	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		i = 0;
		do {
			memset(&std, 0xff, sizeof(std));
			std.index = i;
			enum_ret = ioctl(f, VIDIOC_ENUMSTD, &std);

			dprintf("ENUMSTD: i=%u, enum_ret=%i, errno=%i\n", i, enum_ret, errno);

			if (enum_ret == 0) {
				ret = do_set_video_standard(f, std.id);
				CU_ASSERT_EQUAL(ret, 0);

				dprintf("std.id=0x%llX, ret=%i\n", std.id, ret);
			}
			i++;
		} while (enum_ret == 0 && i != 0);

		/* Setting the original std_id should not fail */
		ret = do_set_video_standard(f, std_id_orig);
		CU_ASSERT_EQUAL(ret, 0);
	}

}


void test_VIDIOC_S_STD_invalid_standard() {
	int ret;
	v4l2_std_id std_id_orig;
	v4l2_std_id std_id;
	int f;

	f = get_video_fd();

	memset(&std_id_orig, 0xff, sizeof(std_id_orig));
	ret = ioctl(f, VIDIOC_G_STD, &std_id_orig);
	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		std_id = 1;
		while (std_id != 0) {
			if (!valid_v4l2_std_id(std_id)) {
				ret = do_set_video_standard(f, std_id);
				CU_ASSERT_EQUAL(ret, -1);
				CU_ASSERT_EQUAL(errno, EINVAL);
				dprintf("ret=%i, errno=%i\n", ret, errno);
			}
			std_id = std_id<<1;
		}

		/* Setting the original std_id should not fail */
		ret = do_set_video_standard(f, std_id_orig);
		CU_ASSERT_EQUAL(ret, 0);
	}
}

void test_VIDIOC_G_STD_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_G_STD, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}

void test_VIDIOC_S_STD_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_S_STD, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}


/* TODO: VIDIOC_S_STD while STREAM_ON */
