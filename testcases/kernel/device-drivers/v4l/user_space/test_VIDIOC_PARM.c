/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 29 Mar 2009  0.2  Comments updated
 * 18 Mar 2009  0.1  First release
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

#include "test_VIDIOC_PARM.h"

int valid_v4l2_captureparm_capability(__u32 capability)
{
	int valid = 0;

	if ((capability & ~(V4L2_CAP_TIMEPERFRAME)) == 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

int valid_v4l2_outputparm_capability(__u32 capability)
{
	int valid = 0;

	if ((capability & ~(V4L2_CAP_TIMEPERFRAME)) == 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

int valid_v4l2_captureparm_capturemode(__u32 capturemode)
{
	int valid = 0;

	if ((capturemode & ~(V4L2_MODE_HIGHQUALITY)) == 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

int valid_v4l2_outputparm_outputpmode(__u32 outputmode)
{
	int valid = 0;

	if ((outputmode & ~(V4L2_MODE_HIGHQUALITY)) == 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

static void do_get_param(enum v4l2_buf_type type)
{
	int ret_get, errno_get;
	struct v4l2_streamparm parm;
	struct v4l2_streamparm parm2;

	memset(&parm, 0xff, sizeof(parm));
	parm.type = type;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_PARM, &parm);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_PARM, type=%i, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, type, ret_get, errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT_EQUAL(parm.type, type);

		switch (parm.type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			dprintf("\t%s:%u: { .type=%i, parm.capture = { "
				".capability = 0x%X, "
				".capturemode = 0x%X, "
				".timeperframe = { .numerator = %u, .denominator = %u }, "
				".extendedmode = %u, "
				".readbuffers = %u, "
				"reserved[] = { 0x%X, 0x%X, 0x%X, 0x%X }}}\n",
				__FILE__, __LINE__,
				parm.type,
				parm.parm.capture.capability,
				parm.parm.capture.capturemode,
				parm.parm.capture.timeperframe.numerator,
				parm.parm.capture.timeperframe.denominator,
				parm.parm.capture.extendedmode,
				parm.parm.capture.readbuffers,
				parm.parm.capture.reserved[0],
				parm.parm.capture.reserved[1],
				parm.parm.capture.reserved[2],
				parm.parm.capture.reserved[3]
			    );

			CU_ASSERT(valid_v4l2_captureparm_capability
				  (parm.parm.capture.capability));
			CU_ASSERT(valid_v4l2_captureparm_capturemode
				  (parm.parm.capture.capturemode));

			if (parm.parm.capture.
			    capability & V4L2_CAP_TIMEPERFRAME) {
				//CU_ASSERT_EQUAL(parm.parm.capture.timeperframe.numerator, ???);
				//CU_ASSERT_EQUAL(parm.parm.capture.timeperframe.denominator, ???);
				CU_ASSERT(parm.parm.capture.timeperframe.
					  denominator != 0);
				// TODO: timerperframe: check struct v4l2_standard frameperiod field
			} else {
				//CU_ASSERT_EQUAL(parm.parm.capture.timeperframe.numerator, 0);
				//CU_ASSERT_EQUAL(parm.parm.capture.timeperframe.denominator, 0);
				CU_ASSERT(parm.parm.output.timeperframe.
					  denominator != 0);
			}

			//CU_ASSERT_EQUAL(parm.parm.capture.extendedmode, ???);
			//CU_ASSERT_EQUAL(parm.parm.capture.readbuffers, ???);

			CU_ASSERT_EQUAL(parm.parm.capture.reserved[0], 0);
			CU_ASSERT_EQUAL(parm.parm.capture.reserved[1], 0);
			CU_ASSERT_EQUAL(parm.parm.capture.reserved[2], 0);
			CU_ASSERT_EQUAL(parm.parm.capture.reserved[3], 0);
			break;
		case V4L2_BUF_TYPE_VIDEO_OUTPUT:
			dprintf("\t%s:%u: { .type=%i, parm.output = { "
				".capability = 0x%X, "
				".outputmode = 0x%X, "
				".timeperframe = { .numerator = %u, .denominator = %u }, "
				".extendedmode = %u, "
				".writebuffers = %u, "
				"reserved[] = { 0x%X, 0x%X, 0x%X, 0x%X }}}\n",
				__FILE__, __LINE__,
				parm.type,
				parm.parm.output.capability,
				parm.parm.output.outputmode,
				parm.parm.output.timeperframe.numerator,
				parm.parm.output.timeperframe.denominator,
				parm.parm.output.extendedmode,
				parm.parm.output.writebuffers,
				parm.parm.output.reserved[0],
				parm.parm.output.reserved[1],
				parm.parm.output.reserved[2],
				parm.parm.output.reserved[3]
			    );

			CU_ASSERT(valid_v4l2_outputparm_capability
				  (parm.parm.output.capability));
			CU_ASSERT(valid_v4l2_outputparm_outputpmode
				  (parm.parm.output.outputmode));

			if (parm.parm.output.capability & V4L2_CAP_TIMEPERFRAME) {
				//CU_ASSERT_EQUAL(parm.parm.output.timeperframe.numerator, ???);
				//CU_ASSERT_EQUAL(parm.parm.output.timeperframe.denominator, ???);
				CU_ASSERT(parm.parm.output.timeperframe.
					  denominator != 0);
				// TODO: timerperframe: check struct v4l2_standard frameperiod field
			} else {
				//CU_ASSERT_EQUAL(parm.parm.output.timeperframe.numerator, 0);
				//CU_ASSERT_EQUAL(parm.parm.output.timeperframe.denominator, 0);
				CU_ASSERT(parm.parm.output.timeperframe.
					  denominator != 0);
			}

			//CU_ASSERT_EQUAL(parm.parm.output.extendedmode, ???);
			//CU_ASSERT_EQUAL(parm.parm.output.writebuffers, ???);

			CU_ASSERT_EQUAL(parm.parm.output.reserved[0], 0);
			CU_ASSERT_EQUAL(parm.parm.output.reserved[1], 0);
			CU_ASSERT_EQUAL(parm.parm.output.reserved[2], 0);
			CU_ASSERT_EQUAL(parm.parm.output.reserved[3], 0);
			break;
		default:
			;
		}

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

		/* check whether the parm structure is untouched */
		memset(&parm2, 0xff, sizeof(parm2));
		parm2.type = type;

		CU_ASSERT_EQUAL(memcmp(&parm, &parm2, sizeof(parm)), 0);

	}

}

void test_VIDIOC_G_PARM()
{
	do_get_param(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_get_param(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_get_param(V4L2_BUF_TYPE_PRIVATE);
}

static void do_get_param_invalid(enum v4l2_buf_type type)
{
	int ret_get, errno_get;
	struct v4l2_streamparm parm;
	struct v4l2_streamparm parm2;

	memset(&parm, 0xff, sizeof(parm));
	parm.type = type;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_PARM, &parm);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_PARM, type=%i, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, type, ret_get, errno_get);

	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);

	/* check whether the parm structure is untouched */
	memset(&parm2, 0xff, sizeof(parm2));
	parm2.type = type;

	CU_ASSERT_EQUAL(memcmp(&parm, &parm2, sizeof(parm)), 0);
}

void test_VIDIOC_G_PARM_invalid()
{
	do_get_param_invalid(S32_MIN);

	/* check if 0x80000001 is not treated as 1 (V4L2_BUF_TYPE_VIDEO_CAPTURE) */
	do_get_param_invalid(S32_MIN + 1);

	/* check if 0x80000002 is not treated as 2 (V4L2_BUF_TYPE_VIDEO_OUTPUT) */
	do_get_param_invalid(S32_MIN + 2);

	do_get_param_invalid(S16_MIN);
	do_get_param_invalid(0);
	do_get_param_invalid(V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_get_param_invalid(V4L2_BUF_TYPE_VBI_CAPTURE);
	do_get_param_invalid(V4L2_BUF_TYPE_VBI_OUTPUT);
	do_get_param_invalid(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_get_param_invalid(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
	do_get_param_invalid(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_get_param_invalid(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY + 1);
	do_get_param_invalid(V4L2_BUF_TYPE_PRIVATE - 1);
	do_get_param_invalid(S16_MAX);
	do_get_param_invalid(S32_MAX);
}

void test_VIDIOC_G_PARM_NULL()
{
	int ret_capture, errno_capture;
	int ret_output, errno_output;
	int ret_private, errno_private;
	int ret_null, errno_null;
	enum v4l2_buf_type type;
	struct v4l2_streamparm parm;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	memset(&parm, 0, sizeof(parm));
	parm.type = type;
	ret_capture = ioctl(get_video_fd(), VIDIOC_G_PARM, &parm);
	errno_capture = errno;
	dprintf
	    ("\t%s:%u: VIDIOC_G_PARM, type=%i, ret_capture=%i, errno_capture=%i\n",
	     __FILE__, __LINE__, type, ret_capture, errno_capture);

	type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	memset(&parm, 0, sizeof(parm));
	parm.type = type;
	ret_output = ioctl(get_video_fd(), VIDIOC_G_PARM, &parm);
	errno_output = errno;
	dprintf
	    ("\t%s:%u: VIDIOC_G_PARM, type=%i, ret_output=%i, errno_output=%i\n",
	     __FILE__, __LINE__, type, ret_output, errno_output);

	type = V4L2_BUF_TYPE_PRIVATE;
	memset(&parm, 0, sizeof(parm));
	parm.type = type;
	ret_private = ioctl(get_video_fd(), VIDIOC_G_PARM, &parm);
	errno_private = errno;
	dprintf
	    ("\t%s:%u: VIDIOC_G_PARM, type=%i, ret_private=%i, errno_private=%i\n",
	     __FILE__, __LINE__, type, ret_private, errno_private);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_PARM, NULL);
	errno_null = errno;
	dprintf("\t%s:%u: VIDIOC_G_PARM, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_capture == 0 || ret_output == 0 || ret_private == 0) {
		/* if at least one type is supported, then the
		 * parameter shall be tested and the result shall be EFAULT
		 */
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_capture, -1);
		CU_ASSERT_EQUAL(errno_capture, EINVAL);
		CU_ASSERT_EQUAL(ret_output, -1);
		CU_ASSERT_EQUAL(errno_output, EINVAL);
		CU_ASSERT_EQUAL(ret_private, -1);
		CU_ASSERT_EQUAL(errno_private, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}

/* TODO: test cases for VIDIOC_S_PARM */
