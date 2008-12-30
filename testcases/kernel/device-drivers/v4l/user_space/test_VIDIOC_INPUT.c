/*
 * v4l-test: Test environment for Video For Linux Two API
 *
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

#include "test_VIDIOC_INPUT.h"

int valid_input_index(int f, __u32 index) {
	__u32 i;
	struct v4l2_input input;
	int ret;
	int valid = 0;

	/* Search for index with VIDIOC_ENUMINPUT. Do not just
	 * check the given index with VIDIOC_ENUMINPUT because
	 * we could miss the end of the enumeration. After the
	 * end of enumeration no more indexes are allowed.
	 */
	i = 0;
	do {
		memset(&input, 0xff, sizeof(input));
		input.index = i;
		ret = ioctl(f, VIDIOC_ENUMINPUT, &input);
		if (ret == 0 && index == i) {
			valid = 1;
			break;
		}
		i++;
	} while (ret == 0 && i != 0);
	return valid;
}

void test_VIDIOC_G_INPUT() {
	int ret;
	__u32 index;
	int f;

	f = get_video_fd();

	memset(&index, 0xff, sizeof(index));
	ret = ioctl(f, VIDIOC_G_INPUT, &index);

	dprintf("VIDIOC_G_INPUT, ret=%i\n", ret);

	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		CU_ASSERT(valid_input_index(f, index));

		dprintf("index=0x%X\n", index);

	}

}

void test_VIDIOC_S_INPUT_from_enum() {
	int ret;
	int enum_ret;
	__u32 input_index_orig;
	struct v4l2_input input;
	__u32 i;
	int f;

	f = get_video_fd();

	memset(&input_index_orig, 0xff, sizeof(input_index_orig));
	ret = ioctl(f, VIDIOC_G_INPUT, &input_index_orig);
	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		i = 0;
		do {
			memset(&input, 0xff, sizeof(input));
			input.index = i;
			enum_ret = ioctl(f, VIDIOC_ENUMINPUT, &input);

			dprintf("ENUMINPUT: i=%u, enum_ret=%i, errno=%i\n", i, enum_ret, errno);

			if (enum_ret == 0) {
				ret = ioctl(f, VIDIOC_S_INPUT, &input.index);
				CU_ASSERT_EQUAL(ret, 0);

				dprintf("input.index=0x%X, ret=%i, errno=%i\n", input.index, ret, errno);

			}
			i++;
		} while (enum_ret == 0 && i != 0);

		/* Setting the original input_id should not fail */
		ret = ioctl(f, VIDIOC_S_INPUT, &input_index_orig);
		CU_ASSERT_EQUAL(ret, 0);
	}
}

static void do_set_input(int f, __u32 first_wrong_input, __u32 index) {
	struct v4l2_input input;
	int ret;

	if (first_wrong_input <= index) {

		dprintf("do_set_input(f, 0x%X, 0x%X)\n", first_wrong_input, index);

		memset(&input, 0xff, sizeof(input));
		input.index = index;
		ret = ioctl(f, VIDIOC_S_INPUT, &input.index);
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

		dprintf("input.index=0x%X, ret=%i, errno=%i\n", input.index, ret, errno);

	}

}

void test_VIDIOC_S_INPUT_invalid_inputs() {
	int ret;
	int enum_ret;
	__u32 input_index_orig;
	struct v4l2_input input;
	__u32 i, first_wrong_input;
	int f;

	f = get_video_fd();

	memset(&input_index_orig, 0xff, sizeof(input_index_orig));
	ret = ioctl(f, VIDIOC_G_INPUT, &input_index_orig);
	CU_ASSERT_EQUAL(ret, 0);
	if (ret == 0) {
		i = 0;
		do {
			memset(&input, 0xff, sizeof(input));
			input.index = i;
			enum_ret = ioctl(f, VIDIOC_ENUMINPUT, &input);

			dprintf("ENUMINPUT: i=%u, enum_ret=%i, errno=%i\n", i, enum_ret, errno);

			i++;
		} while (enum_ret == 0 && i != 0);

		if (i != 0) {
			first_wrong_input = i;

			/* The input index range 0..(i-1) are valid inputs. */
			/* Try index values from range i..U32_MAX */
			do_set_input(f, first_wrong_input, i);
			do_set_input(f, first_wrong_input, i+1);

			/* Check for signed/unsigned mismatch near S32_MAX */
			for (i = 0; i <= first_wrong_input+1; i++) {
				do_set_input(f, first_wrong_input, ((__u32)S32_MAX) + i);
			}

			i = (U32_MAX-1)-first_wrong_input;
			do {
				do_set_input(f, first_wrong_input, i);
				i++;
			} while (i != 0);
		}

		/* Setting the original input_id should not fail */
		ret = ioctl(f, VIDIOC_S_INPUT, &input_index_orig);
		CU_ASSERT_EQUAL(ret, 0);
	}
}

void test_VIDIOC_G_INPUT_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_G_INPUT, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}

void test_VIDIOC_S_INPUT_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_S_INPUT, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}
