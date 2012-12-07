/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  4 Apr 2009  0.3  Test case for NULL parameter reworked
 * 29 Mar 2009  0.2  Clean up test case for NULL parameter
 * 22 Mar 2009  0.1  First release
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

#include "test_VIDIOC_OUTPUT.h"

int valid_output_index(int f, __u32 index)
{
	__u32 i;
	struct v4l2_output output;
	int ret_enum, errno_enum;
	int valid = 0;

	/* Search for index with VIDIOC_ENUMOUTPUT. Do not just
	 * check the given index with VIDIOC_ENUMOUTPUT because
	 * we could miss the end of the enumeration. After the
	 * end of enumeration no more indexes are allowed.
	 */
	i = 0;
	do {
		memset(&output, 0xff, sizeof(output));
		output.index = i;
		ret_enum = ioctl(f, VIDIOC_ENUMOUTPUT, &output);
		errno_enum = errno;
		if (ret_enum == 0 && index == i) {
			valid = 1;
			break;
		}
		i++;
	} while (ret_enum == 0 && i != 0);
	return valid;
}

void test_VIDIOC_G_OUTPUT()
{
	int ret_get, errno_get;
	__u32 index;
	int f;

	f = get_video_fd();

	memset(&index, 0xff, sizeof(index));
	ret_get = ioctl(f, VIDIOC_G_OUTPUT, &index);
	errno_get = errno;

	dprintf("\tVIDIOC_G_OUTPUT, ret_get=%i, errno_get=%i\n", ret_get,
		errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT(valid_output_index(f, index));

		dprintf("\tindex=0x%X\n", index);

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}

}

void test_VIDIOC_S_OUTPUT_from_enum()
{
	int ret_get, errno_get;
	int ret_set, errno_set;
	int ret_enum, errno_enum;
	__u32 output_index_orig;
	struct v4l2_output output;
	__u32 i;
	int f;

	f = get_video_fd();

	memset(&output_index_orig, 0xff, sizeof(output_index_orig));
	ret_get = ioctl(f, VIDIOC_G_OUTPUT, &output_index_orig);
	errno_get = errno;
	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		i = 0;
		do {
			memset(&output, 0xff, sizeof(output));
			output.index = i;
			ret_enum = ioctl(f, VIDIOC_ENUMOUTPUT, &output);
			errno_enum = errno;

			dprintf
			    ("\tENUMOUTPUT: i=%u, ret_enum=%i, errno_enum=%i\n",
			     i, ret_enum, errno_enum);

			if (ret_enum == 0) {
				ret_set =
				    ioctl(f, VIDIOC_S_OUTPUT, &output.index);
				errno_set = errno;
				CU_ASSERT_EQUAL(ret_set, 0);

				dprintf
				    ("\toutput.index=0x%X, ret_set=%i, errno_set=%i\n",
				     output.index, ret_set, errno_set);

			}
			i++;
		} while (ret_enum == 0 && i != 0);

		/* Setting the original output_id should not fail */
		ret_set = ioctl(f, VIDIOC_S_OUTPUT, &output_index_orig);
		errno_set = errno;
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}
}

static void do_set_output(int f, __u32 first_wrong_output, __u32 index)
{
	struct v4l2_output output;
	int ret_set, errno_set;

	if (first_wrong_output <= index) {

		dprintf("\tdo_set_output(f, 0x%X, 0x%X)\n", first_wrong_output,
			index);

		memset(&output, 0xff, sizeof(output));
		output.index = index;
		ret_set = ioctl(f, VIDIOC_S_OUTPUT, &output.index);
		errno_set = errno;

		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

		dprintf("\toutput.index=0x%X, ret_set=%i, errno_set=%i\n",
			output.index, ret_set, errno_set);

	}

}

void test_VIDIOC_S_OUTPUT_invalid_outputs()
{
	int ret_get, errno_get;
	int ret_set, errno_set;
	int ret_enum, errno_enum;
	__u32 output_index_orig;
	struct v4l2_output output;
	__u32 i, first_wrong_output;
	int f;

	f = get_video_fd();

	memset(&output_index_orig, 0xff, sizeof(output_index_orig));
	ret_get = ioctl(f, VIDIOC_G_OUTPUT, &output_index_orig);
	errno_get = errno;
	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		i = 0;
		do {
			memset(&output, 0xff, sizeof(output));
			output.index = i;
			ret_enum = ioctl(f, VIDIOC_ENUMOUTPUT, &output);
			errno_enum = errno;

			dprintf
			    ("\tENUMOUTPUT: i=%u, ret_enum=%i, errno_enum=%i\n",
			     i, ret_enum, errno_enum);

			i++;
		} while (ret_enum == 0 && i != 0);

		if (i != 0) {
			first_wrong_output = i;

			/* The output index range 0..(i-1) are valid outputs. */
			/* Try index values from range i..U32_MAX */
			do_set_output(f, first_wrong_output, i);
			do_set_output(f, first_wrong_output, i + 1);

			/* Check for signed/unsigned mismatch near S32_MAX */
			for (i = 0; i <= first_wrong_output + 1; i++) {
				do_set_output(f, first_wrong_output,
					      ((__u32) S32_MAX) + i);
			}

			i = (U32_MAX - 1) - first_wrong_output;
			do {
				do_set_output(f, first_wrong_output, i);
				i++;
			} while (i != 0);
		}

		/* Setting the original output_id should not fail */
		ret_set = ioctl(f, VIDIOC_S_OUTPUT, &output_index_orig);
		errno_set = errno;
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}
}

void test_VIDIOC_G_OUTPUT_NULL()
{
	int ret_get, errno_get;
	int ret_null, errno_null;
	__u32 index;

	memset(&index, 0xff, sizeof(index));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_OUTPUT, &index);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_OUTPUT, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_OUTPUT, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_G_OUTPUT: ret_null=%i, errno_null=%i\n",
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

void test_VIDIOC_S_OUTPUT_NULL()
{
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	int ret_null, errno_null;
	__u32 index_orig;
	__u32 index;

	/* save the original output */
	memset(&index_orig, 0, sizeof(index_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_OUTPUT, &index_orig);
	errno_orig = errno;

	dprintf("\t%s:%u: VIDIOC_G_OUTPUT, ret_orig=%i, errno_orig=%i\n",
		__FILE__, __LINE__, ret_orig, errno_orig);

	memset(&index, 0xff, sizeof(index));
	index = index_orig;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_OUTPUT, &index);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_S_OUTPUT ret_set=%i, errno_set=%i\n",
		__FILE__, __LINE__, ret_set, errno_set);

	ret_null = ioctl(get_video_fd(), VIDIOC_S_OUTPUT, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_S_OUTPUT: ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_set == 0) {
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}
