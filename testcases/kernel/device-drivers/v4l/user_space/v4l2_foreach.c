/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  5 Jul 2009  0.1  First release
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
#include "v4l2_show.h"
#include "v4l2_foreach.h"

void foreach_input(V4L2InputTestFunc pFunc)
{
	__u32 input_index_orig;
	struct v4l2_input input;
	int ret_input_get, errno_input_get;
	int ret_input_enum, errno_input_enum;
	int ret_input_set, errno_input_set;
	__u32 i;
	int f;
	char not_yet_called = 1;

	f = get_video_fd();

	memset(&input_index_orig, 0xff, sizeof(input_index_orig));
	ret_input_get = ioctl(f, VIDIOC_G_INPUT, &input_index_orig);
	errno_input_get = errno;
	dprintf
	    ("\t%s:%u: VIDIOC_G_INPUT, ret_input_get=%i, errno_input_get=%i, input_index_orig=0x%X\n",
	     __FILE__, __LINE__, ret_input_get, errno_input_get,
	     input_index_orig);

	i = 0;
	do {
		memset(&input, 0xff, sizeof(input));
		input.index = i;
		ret_input_enum = ioctl(f, VIDIOC_ENUMINPUT, &input);
		errno_input_enum = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUMINPUT: i=%u, ret_input_enum=%i, errno_input_enum=%i\n",
		     __FILE__, __LINE__, i, ret_input_enum, errno_input_enum);

		if (ret_input_enum == 0) {
			show_v4l2_input(&input);
			ret_input_set = ioctl(f, VIDIOC_S_INPUT, &input.index);
			errno_input_set = errno;
			dprintf
			    ("\t%s:%u: VIDIOC_S_INPUT: input.index=0x%X, ret_input_set=%i, errno_input_set=%i\n",
			     __FILE__, __LINE__, input.index, ret_input_set,
			     errno_input_set);
			CU_ASSERT_EQUAL(ret_input_set, 0);
		}

		/* Ensure that pFunc() is called at least once even if
		 * everything else returned error before.
		 */
		if (not_yet_called || ret_input_enum == 0) {
			pFunc(ret_input_enum, errno_input_enum, &input);
			not_yet_called = 0;
		}

		i++;
	} while (ret_input_enum == 0 && i != 0);

	if (ret_input_get == 0) {
		/* Setting the original input_id should not fail */
		ret_input_set = ioctl(f, VIDIOC_S_INPUT, &input_index_orig);
		errno_input_set = errno;
		CU_ASSERT_EQUAL(ret_input_set, 0);
	}
}
