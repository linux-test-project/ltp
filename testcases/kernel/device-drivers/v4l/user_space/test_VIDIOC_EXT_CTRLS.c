/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 19 May 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

/*
 * Note: V4L2_CID_LASTP1 != V4L2_CID_BASE_LASTP1
 */

#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include <linux/videodev2.h>
#include <linux/errno.h>

#include <CUnit/CUnit.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_VIDIOC_EXT_CTRLS.h"

void test_VIDIOC_G_EXT_CTRLS_zero()
{
	struct v4l2_ext_controls controls;
	int ret_get, errno_get;

	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = 0;
	controls.controls = NULL;

	ret_get = ioctl(get_video_fd(), VIDIOC_G_EXT_CTRLS, &controls);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_EXT_CTRLS, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		CU_ASSERT_EQUAL(controls.ctrl_class, V4L2_CTRL_CLASS_USER);
		CU_ASSERT_EQUAL(controls.count, 0);
		// The value of controls.error_idx is not defined when ret_get == 0
		CU_ASSERT_EQUAL(controls.reserved[0], 0);
		CU_ASSERT_EQUAL(controls.reserved[1], 0);
		CU_ASSERT_EQUAL(controls.controls, NULL);

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}
}

void test_VIDIOC_G_EXT_CTRLS_zero_invalid_count()
{
	struct v4l2_ext_controls controls;
	int ret_get, errno_get;
	int ret_get_invalid, errno_get_invalid;
	__u32 count;

	count = 0;
	memset(&controls, 0, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = count;
	controls.controls = NULL;

	ret_get = ioctl(get_video_fd(), VIDIOC_G_EXT_CTRLS, &controls);
	errno_get = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_EXT_CTRLS, count=0%x, ret_get=%i, errno_get=%i\n",
	     __FILE__, __LINE__, count, ret_get, errno_get);

	count = 1;
	/* Note: this loop also covers ((__u32)S32_MAX)+1 = 0x80000000 */
	do {
		memset(&controls, 0xff, sizeof(controls));
		controls.ctrl_class = V4L2_CTRL_CLASS_USER;
		controls.count = count;
		controls.controls = NULL;

		ret_get_invalid =
		    ioctl(get_video_fd(), VIDIOC_G_EXT_CTRLS, &controls);
		errno_get_invalid = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_G_EXT_CTRLS, count=0x%x, ret_get_invalid=%i, errno_get_invalid=%i\n",
		     __FILE__, __LINE__, count, ret_get_invalid,
		     errno_get_invalid);

		if (ret_get == 0) {
			CU_ASSERT_EQUAL(ret_get, 0);

			/* if the VIDIOC_G_EXT_CTRLS is supported by the driver
			 * it shall complain about the NULL pointer at
			 * cotrols.controls because this does not match the
			 * controls.count value
			 */
			CU_ASSERT_EQUAL(ret_get_invalid, -1);
			CU_ASSERT(errno_get_invalid == EFAULT
				  || errno_get_invalid == ENOMEM);

		} else {
			CU_ASSERT_EQUAL(ret_get, -1);
			CU_ASSERT_EQUAL(errno_get, EINVAL);

			CU_ASSERT_EQUAL(ret_get_invalid, -1);
			CU_ASSERT_EQUAL(errno_get_invalid, EINVAL);
		}
		count <<= 1;
	} while (count != 0);

	count = (__u32) S32_MAX;
	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = count;
	controls.controls = NULL;

	ret_get_invalid = ioctl(get_video_fd(), VIDIOC_G_EXT_CTRLS, &controls);
	errno_get_invalid = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_EXT_CTRLS, count=0x%x, ret_get_invalid=%i, errno_get_invalid=%i\n",
	     __FILE__, __LINE__, count, ret_get_invalid, errno_get_invalid);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		CU_ASSERT_EQUAL(ret_get_invalid, -1);
		CU_ASSERT(errno_get_invalid == EFAULT
			  || errno_get_invalid == ENOMEM);

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

		CU_ASSERT_EQUAL(ret_get_invalid, -1);
		CU_ASSERT_EQUAL(errno_get_invalid, EINVAL);
	}

	count = U32_MAX;
	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = count;
	controls.controls = NULL;

	ret_get_invalid = ioctl(get_video_fd(), VIDIOC_G_EXT_CTRLS, &controls);
	errno_get_invalid = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_EXT_CTRLS, count=0x%x, ret_get_invalid=%i, errno_get_invalid=%i\n",
	     __FILE__, __LINE__, count, ret_get_invalid, errno_get_invalid);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		CU_ASSERT_EQUAL(ret_get_invalid, -1);
		CU_ASSERT(errno_get_invalid == EFAULT
			  || errno_get_invalid == ENOMEM);

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

		CU_ASSERT_EQUAL(ret_get_invalid, -1);
		CU_ASSERT_EQUAL(errno_get_invalid, EINVAL);
	}

}

static int do_get_ext_control_one(__u32 ctrl_class, __u32 id)
{
	int ret_query, errno_query;
	int ret_get, errno_get;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_ext_controls controls;
	struct v4l2_ext_control control;

	/* The expected return value of VIDIOC_G_EXT_CTRLS depens on the value
	 * reported by VIDIOC_QUERYCTRL
	 */

	memset(&queryctrl, 0, sizeof(queryctrl));
	queryctrl.id = id;
	ret_query = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
	errno_query = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_QUERYCTRL, id=%u (V4L2_CID_BASE+%i), ret_query=%i, errno_query=%i\n",
	     __FILE__, __LINE__, id, id - V4L2_CID_BASE, ret_query,
	     errno_query);
	if (ret_query == 0) {
		dprintf("\t%s:%u: queryctrl = {.id=%u, .type=%i, .name=\"%s\", "
			".minimum=%i, .maximum=%i, .step=%i, "
			".default_value=%i, "
			".flags=0x%X, "
			".reserved[]={ 0x%X, 0x%X } }\n",
			__FILE__, __LINE__,
			queryctrl.id,
			queryctrl.type,
			queryctrl.name,
			queryctrl.minimum,
			queryctrl.maximum,
			queryctrl.step,
			queryctrl.default_value,
			queryctrl.flags,
			queryctrl.reserved[0], queryctrl.reserved[1]
		    );
	}

	memset(&control, 0xff, sizeof(control));
	control.id = id;

	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = ctrl_class;
	controls.count = 1;
	controls.controls = &control;

	ret_get = ioctl(get_video_fd(), VIDIOC_G_EXT_CTRLS, &controls);
	errno_get = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_EXT_CTRLS, id=%u (V4L2_CID_BASE+%i), ret_get=%i, errno_get=%i\n",
	     __FILE__, __LINE__, id, id - V4L2_CID_BASE, ret_get, errno_get);

	if (ret_query == 0) {
		CU_ASSERT_EQUAL(ret_query, 0);

		switch (queryctrl.type) {
		case V4L2_CTRL_TYPE_INTEGER:
		case V4L2_CTRL_TYPE_BOOLEAN:
		case V4L2_CTRL_TYPE_MENU:
			if (ret_get == 0) {
				CU_ASSERT_EQUAL(ret_get, 0);

				CU_ASSERT(queryctrl.minimum <= control.value);
				CU_ASSERT(control.value <= queryctrl.maximum);
			} else {
				/* This is the case when VIDIOC_G_CTRLS is not
				 * supported at all.
				 */
				CU_ASSERT_EQUAL(ret_get, -1);
				CU_ASSERT_EQUAL(errno_get, EINVAL);
			}
			break;

		case V4L2_CTRL_TYPE_BUTTON:
			/* This control only performs an action, does not have
			 * any value
			 */
			CU_ASSERT_EQUAL(ret_get, -1);
			CU_ASSERT_EQUAL(errno_get, EINVAL);
			break;

		case V4L2_CTRL_TYPE_INTEGER64:	/* TODO: what about this case? */
		case V4L2_CTRL_TYPE_CTRL_CLASS:
		default:
			CU_ASSERT_EQUAL(ret_get, -1);
			CU_ASSERT_EQUAL(errno_get, EINVAL);
		}
	} else {
		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);

		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

	}

	return ret_query;
}

void test_VIDIOC_G_EXT_CTRLS_one()
{
	int ret1;
	__u32 i;

	for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {
		ret1 = do_get_ext_control_one(V4L2_CTRL_CLASS_USER, i);
	}

	ret1 = do_get_ext_control_one(V4L2_CTRL_CLASS_USER, V4L2_CID_BASE - 1);
	ret1 = do_get_ext_control_one(V4L2_CTRL_CLASS_USER, V4L2_CID_LASTP1);
	ret1 =
	    do_get_ext_control_one(V4L2_CTRL_CLASS_USER,
				   V4L2_CID_PRIVATE_BASE - 1);

	i = V4L2_CID_PRIVATE_BASE;
	do {
		ret1 = do_get_ext_control_one(V4L2_CTRL_CLASS_USER, i);
		i++;
	} while (ret1 == 0);

	ret1 = do_get_ext_control_one(V4L2_CTRL_CLASS_USER, i);
}

void test_VIDIOC_G_EXT_CTRLS_NULL()
{
	struct v4l2_ext_controls controls;
	int ret_get, errno_get;
	int ret_null, errno_null;

	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = 0;
	controls.controls = NULL;

	ret_get = ioctl(get_video_fd(), VIDIOC_G_EXT_CTRLS, &controls);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_EXT_CTRLS, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_EXT_CTRLS, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_G_EXT_CTRLS, ret_null=%i, errno_null=%i\n",
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

void test_VIDIOC_S_EXT_CTRLS_zero()
{
	struct v4l2_ext_controls controls;
	int ret_set, errno_set;

	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = 0;
	controls.controls = NULL;

	ret_set = ioctl(get_video_fd(), VIDIOC_S_EXT_CTRLS, &controls);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_S_EXT_CTRLS, ret_set=%i, errno_set=%i\n",
		__FILE__, __LINE__, ret_set, errno_set);

	if (ret_set == 0) {
		CU_ASSERT_EQUAL(ret_set, 0);

		CU_ASSERT_EQUAL(controls.ctrl_class, V4L2_CTRL_CLASS_USER);
		CU_ASSERT_EQUAL(controls.count, 0);
		// The value of controls.error_idx is not defined when ret_set == 0
		CU_ASSERT_EQUAL(controls.reserved[0], 0);
		CU_ASSERT_EQUAL(controls.reserved[1], 0);
		CU_ASSERT_EQUAL(controls.controls, NULL);

	} else {
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}
}

void test_VIDIOC_S_EXT_CTRLS_zero_invalid_count()
{
	struct v4l2_ext_controls controls;
	int ret_set, errno_set;
	int ret_set_invalid, errno_set_invalid;
	__u32 count;

	count = 0;
	memset(&controls, 0, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = count;
	controls.controls = NULL;

	ret_set = ioctl(get_video_fd(), VIDIOC_S_EXT_CTRLS, &controls);
	errno_set = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_S_EXT_CTRLS, count=0%x, ret_set=%i, errno_set=%i\n",
	     __FILE__, __LINE__, count, ret_set, errno_set);

	count = 1;
	/* Note: this loop also covers ((__u32)S32_MAX)+1 = 0x80000000 */
	do {
		memset(&controls, 0xff, sizeof(controls));
		controls.ctrl_class = V4L2_CTRL_CLASS_USER;
		controls.count = count;
		controls.controls = NULL;

		ret_set_invalid =
		    ioctl(get_video_fd(), VIDIOC_S_EXT_CTRLS, &controls);
		errno_set_invalid = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_S_EXT_CTRLS, count=0x%x, ret_set_invalid=%i, errno_set_invalid=%i\n",
		     __FILE__, __LINE__, count, ret_set_invalid,
		     errno_set_invalid);

		if (ret_set == 0) {
			CU_ASSERT_EQUAL(ret_set, 0);

			/* if the VIDIOC_S_EXT_CTRLS is supported by the driver
			 * it shall complain about the NULL pointer at
			 * cotrols.controls because this does not match the
			 * controls.count value
			 */
			CU_ASSERT_EQUAL(ret_set_invalid, -1);
			CU_ASSERT(errno_set_invalid == EFAULT
				  || errno_set_invalid == ENOMEM);

		} else {
			CU_ASSERT_EQUAL(ret_set, -1);
			CU_ASSERT_EQUAL(errno_set, EINVAL);

			CU_ASSERT_EQUAL(ret_set_invalid, -1);
			CU_ASSERT_EQUAL(errno_set_invalid, EINVAL);
		}
		count <<= 1;
	} while (count != 0);

	count = (__u32) S32_MAX;
	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = count;
	controls.controls = NULL;

	ret_set_invalid = ioctl(get_video_fd(), VIDIOC_S_EXT_CTRLS, &controls);
	errno_set_invalid = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_S_EXT_CTRLS, count=0x%x, ret_set_invalid=%i, errno_set_invalid=%i\n",
	     __FILE__, __LINE__, count, ret_set_invalid, errno_set_invalid);

	if (ret_set == 0) {
		CU_ASSERT_EQUAL(ret_set, 0);

		CU_ASSERT_EQUAL(ret_set_invalid, -1);
		CU_ASSERT(errno_set_invalid == EFAULT
			  || errno_set_invalid == ENOMEM);

	} else {
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

		CU_ASSERT_EQUAL(ret_set_invalid, -1);
		CU_ASSERT_EQUAL(errno_set_invalid, EINVAL);
	}

	count = U32_MAX;
	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = count;
	controls.controls = NULL;

	ret_set_invalid = ioctl(get_video_fd(), VIDIOC_S_EXT_CTRLS, &controls);
	errno_set_invalid = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_S_EXT_CTRLS, count=0x%x, ret_set_invalid=%i, errno_set_invalid=%i\n",
	     __FILE__, __LINE__, count, ret_set_invalid, errno_set_invalid);

	if (ret_set == 0) {
		CU_ASSERT_EQUAL(ret_set, 0);

		CU_ASSERT_EQUAL(ret_set_invalid, -1);
		CU_ASSERT(errno_set_invalid == EFAULT
			  || errno_set_invalid == ENOMEM);

	} else {
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

		CU_ASSERT_EQUAL(ret_set_invalid, -1);
		CU_ASSERT_EQUAL(errno_set_invalid, EINVAL);
	}

}

void test_VIDIOC_S_EXT_CTRLS_NULL()
{
	struct v4l2_ext_controls controls;
	int ret_set, errno_set;
	int ret_null, errno_null;

	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = 0;
	controls.controls = NULL;

	ret_set = ioctl(get_video_fd(), VIDIOC_S_EXT_CTRLS, &controls);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_S_EXT_CTRLS, ret_set=%i, errno_set=%i\n",
		__FILE__, __LINE__, ret_set, errno_set);

	ret_null = ioctl(get_video_fd(), VIDIOC_S_EXT_CTRLS, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_S_EXT_CTRLS, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_set == 0) {
		CU_ASSERT_EQUAL(ret_set, 0);

		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);

	} else {
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}
}

void test_VIDIOC_TRY_EXT_CTRLS_zero()
{
	struct v4l2_ext_controls controls;
	int ret_try, errno_try;

	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = 0;
	controls.controls = NULL;

	ret_try = ioctl(get_video_fd(), VIDIOC_TRY_EXT_CTRLS, &controls);
	errno_try = errno;

	dprintf("\t%s:%u: VIDIOC_TRY_EXT_CTRLS, ret_try=%i, errno_try=%i\n",
		__FILE__, __LINE__, ret_try, errno_try);

	if (ret_try == 0) {
		CU_ASSERT_EQUAL(ret_try, 0);

		CU_ASSERT_EQUAL(controls.ctrl_class, V4L2_CTRL_CLASS_USER);
		CU_ASSERT_EQUAL(controls.count, 0);
		// The value of controls.error_idx is not defined when ret_try == 0
		CU_ASSERT_EQUAL(controls.reserved[0], 0);
		CU_ASSERT_EQUAL(controls.reserved[1], 0);
		CU_ASSERT_EQUAL(controls.controls, NULL);

	} else {
		CU_ASSERT_EQUAL(ret_try, -1);
		CU_ASSERT_EQUAL(errno_try, EINVAL);
	}
}

void test_VIDIOC_TRY_EXT_CTRLS_zero_invalid_count()
{
	struct v4l2_ext_controls controls;
	int ret_try, errno_try;
	int ret_try_invalid, errno_try_invalid;
	__u32 count;

	count = 0;
	memset(&controls, 0, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = count;
	controls.controls = NULL;

	ret_try = ioctl(get_video_fd(), VIDIOC_TRY_EXT_CTRLS, &controls);
	errno_try = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_TRY_EXT_CTRLS, count=0x%x, ret_try=%i, errno_try=%i\n",
	     __FILE__, __LINE__, count, ret_try, errno_try);

	count = 1;
	/* Note: this loop also covers ((__u32)S32_MAX)+1 = 0x80000000 */
	do {
		memset(&controls, 0xff, sizeof(controls));
		controls.ctrl_class = V4L2_CTRL_CLASS_USER;
		controls.count = count;
		controls.controls = NULL;

		ret_try_invalid =
		    ioctl(get_video_fd(), VIDIOC_TRY_EXT_CTRLS, &controls);
		errno_try_invalid = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_TRY_EXT_CTRLS, count=0x%x, ret_try_invalid=%i, errno_try_invalid=%i\n",
		     __FILE__, __LINE__, count, ret_try_invalid,
		     errno_try_invalid);

		if (ret_try == 0) {
			CU_ASSERT_EQUAL(ret_try, 0);

			/* if the VIDIOC_TRY_EXT_CTRLS is supported by the driver
			 * it shall complain about the NULL pointer at
			 * cotrols.controls because this does not match the
			 * controls.count value
			 */
			CU_ASSERT_EQUAL(ret_try_invalid, -1);
			CU_ASSERT(errno_try_invalid == EFAULT
				  || errno_try_invalid == ENOMEM);

		} else {
			CU_ASSERT_EQUAL(ret_try, -1);
			CU_ASSERT_EQUAL(errno_try, EINVAL);

			CU_ASSERT_EQUAL(ret_try_invalid, -1);
			CU_ASSERT_EQUAL(errno_try_invalid, EINVAL);
		}
		count <<= 1;
	} while (count != 0);

	count = (__u32) S32_MAX;
	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = count;
	controls.controls = NULL;

	ret_try_invalid =
	    ioctl(get_video_fd(), VIDIOC_TRY_EXT_CTRLS, &controls);
	errno_try_invalid = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_TRY_EXT_CTRLS, count=0x%x, ret_try_invalid=%i, errno_try_invalid=%i\n",
	     __FILE__, __LINE__, count, ret_try_invalid, errno_try_invalid);

	if (ret_try == 0) {
		CU_ASSERT_EQUAL(ret_try, 0);

		CU_ASSERT_EQUAL(ret_try_invalid, -1);
		CU_ASSERT(errno_try_invalid == EFAULT
			  || errno_try_invalid == ENOMEM);

	} else {
		CU_ASSERT_EQUAL(ret_try, -1);
		CU_ASSERT_EQUAL(errno_try, EINVAL);

		CU_ASSERT_EQUAL(ret_try_invalid, -1);
		CU_ASSERT_EQUAL(errno_try_invalid, EINVAL);
	}

	count = U32_MAX;
	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = count;
	controls.controls = NULL;

	ret_try_invalid =
	    ioctl(get_video_fd(), VIDIOC_TRY_EXT_CTRLS, &controls);
	errno_try_invalid = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_TRY_EXT_CTRLS, count=0x%x, ret_try_invalid=%i, errno_try_invalid=%i\n",
	     __FILE__, __LINE__, count, ret_try_invalid, errno_try_invalid);

	if (ret_try == 0) {
		CU_ASSERT_EQUAL(ret_try, 0);

		CU_ASSERT_EQUAL(ret_try_invalid, -1);
		CU_ASSERT(errno_try_invalid == EFAULT
			  || errno_try_invalid == ENOMEM);

	} else {
		CU_ASSERT_EQUAL(ret_try, -1);
		CU_ASSERT_EQUAL(errno_try, EINVAL);

		CU_ASSERT_EQUAL(ret_try_invalid, -1);
		CU_ASSERT_EQUAL(errno_try_invalid, EINVAL);
	}

}

void test_VIDIOC_TRY_EXT_CTRLS_NULL()
{
	struct v4l2_ext_controls controls;
	int ret_try, errno_try;
	int ret_null, errno_null;

	memset(&controls, 0xff, sizeof(controls));
	controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	controls.count = 0;
	controls.controls = NULL;

	ret_try = ioctl(get_video_fd(), VIDIOC_TRY_EXT_CTRLS, &controls);
	errno_try = errno;

	dprintf("\t%s:%u: VIDIOC_TRY_EXT_CTRLS, ret_try=%i, errno_try=%i\n",
		__FILE__, __LINE__, ret_try, errno_try);

	ret_null = ioctl(get_video_fd(), VIDIOC_TRY_EXT_CTRLS, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_TRY_EXT_CTRLS, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_try == 0) {
		CU_ASSERT_EQUAL(ret_try, 0);

		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);

	} else {
		CU_ASSERT_EQUAL(ret_try, -1);
		CU_ASSERT_EQUAL(errno_try, EINVAL);

		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}
}
