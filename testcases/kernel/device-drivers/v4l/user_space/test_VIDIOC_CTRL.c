/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 25 Mar 2009  0.5  Cleaned up ret and errno variable names
 * 14 Mar 2009  0.4  Added test steps for S16_MIN, S16_MAX, U16_MIN and U16_MAX
 *  6 Mar 2009  0.3  Check whether the newly set value is converted to the
 *                   closest valid value when setting out of bounds value
 * 22 Feb 2009  0.2  Added test cases for VIDIOC_S_CTRL
 * 19 Feb 2009  0.1  First release
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

#include "test_VIDIOC_CTRL.h"

static int do_get_control(__u32 id)
{
	int ret_query, errno_query;
	int ret_get, errno_get;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_control control;

	/* The expected return value of VIDIOC_G_CTRL depens on the value
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
	ret_get = ioctl(get_video_fd(), VIDIOC_G_CTRL, &control);
	errno_get = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_CTRL, id=%u (V4L2_CID_BASE+%i), ret_get=%i, errno_get=%i\n",
	     __FILE__, __LINE__, id, id - V4L2_CID_BASE, ret_get, errno_get);

	if (ret_query == 0) {
		CU_ASSERT_EQUAL(ret_query, 0);

		switch (queryctrl.type) {
		case V4L2_CTRL_TYPE_INTEGER:
		case V4L2_CTRL_TYPE_BOOLEAN:
		case V4L2_CTRL_TYPE_MENU:
			CU_ASSERT_EQUAL(ret_get, 0);
			if (ret_get == 0) {
				CU_ASSERT(queryctrl.minimum <= control.value);
				CU_ASSERT(control.value <= queryctrl.maximum);
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

void test_VIDIOC_G_CTRL()
{
	int ret1;
	__u32 i;

	for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {
		ret1 = do_get_control(i);
	}

	ret1 = do_get_control(V4L2_CID_BASE - 1);
	ret1 = do_get_control(V4L2_CID_LASTP1);
	ret1 = do_get_control(V4L2_CID_PRIVATE_BASE - 1);

	i = V4L2_CID_PRIVATE_BASE;
	do {
		ret1 = do_get_control(i);
		i++;
	} while (ret1 == 0);

	ret1 = do_get_control(i);
}

void test_VIDIOC_G_CTRL_NULL()
{
	int ret_get, errno_get;
	int ret_null, errno_null;
	struct v4l2_control control;
	__u32 id;

	id = V4L2_CID_BASE;
	ret_get = -1;
	while (ret_get == -1 && id < V4L2_CID_LASTP1) {
		memset(&control, 0xff, sizeof(control));
		control.id = id;
		ret_get = ioctl(get_video_fd(), VIDIOC_G_CTRL, &control);
		errno_get = errno;
		dprintf
		    ("\t%s:%u: VIDIOC_G_CTRL, id=%u (V4L2_CID_BASE+%i), ret_get=%i, errno_get=%i\n",
		     __FILE__, __LINE__, id, id - V4L2_CID_BASE, ret_get,
		     errno_get);
	}

	ret_null = ioctl(get_video_fd(), VIDIOC_G_CTRL, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_G_CTRL, ret_null=%i, errno_null=%i\n",
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

int do_set_control(__u32 id)
{
	int ret_query, errno_query;
	int ret_set, errno_set;
	int ret_get, errno_get;
	int ret_orig, errno_orig;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_control control_orig;
	struct v4l2_control control;
	struct v4l2_control control_new;
	__s32 value;

	/* The expected return value of VIDIOC_S_CTRL depens on the value
	 * reported by VIDIOC_QUERYCTRL. The allowed limits are also
	 * reported by VIDIOC_QUERYCTRL.
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

	memset(&control_orig, 0, sizeof(control_orig));
	control_orig.id = id;
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_CTRL, &control_orig);
	errno_orig = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_CTRL, id=%u (V4L2_CID_BASE+%i), ret_orig=%i, errno_orig=%i, control_orig.value=%i\n",
	     __FILE__, __LINE__, id, id - V4L2_CID_BASE, ret_orig, errno_orig,
	     control_orig.value);

	if (ret_query == 0) {
		CU_ASSERT_EQUAL(ret_query, 0);

		switch (queryctrl.type) {
		case V4L2_CTRL_TYPE_INTEGER:
		case V4L2_CTRL_TYPE_BOOLEAN:
		case V4L2_CTRL_TYPE_MENU:

			/* TODO: this is an infinite loop if queryctrl.maximum == S32_MAX */
			for (value = queryctrl.minimum;
			     value <= queryctrl.maximum; value++) {
				memset(&control, 0xff, sizeof(control));
				control.id = id;
				control.value = value;
				ret_set =
				    ioctl(get_video_fd(), VIDIOC_S_CTRL,
					  &control);
				errno_set = errno;

				dprintf
				    ("\t%s:%u: VIDIOC_S_CTRL, id=%u (V4L2_CID_BASE+%i), value=%i, ret_set=%i, errno_set=%i\n",
				     __FILE__, __LINE__, id, id - V4L2_CID_BASE,
				     value, ret_set, errno_set);

				if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
				    queryctrl.
				    flags & V4L2_CTRL_FLAG_READ_ONLY) {
					CU_ASSERT_EQUAL(ret_set, -1);
					CU_ASSERT_EQUAL(errno_set, EINVAL);
				} else if (queryctrl.
					   flags & V4L2_CTRL_FLAG_GRABBED) {
					CU_ASSERT_EQUAL(ret_set, -1);
					CU_ASSERT_EQUAL(errno_set, EBUSY);
				} else {
					CU_ASSERT_EQUAL(ret_set, 0);
				}

				memset(&control_new, 0, sizeof(control_new));
				control_new.id = id;
				ret_get =
				    ioctl(get_video_fd(), VIDIOC_G_CTRL,
					  &control_new);
				errno_get = errno;

				dprintf
				    ("\t%s:%u: VIDIOC_G_CTRL, id=%u (V4L2_CID_BASE+%i), ret_get=%i, errno_get=%i, control_new.value=%i\n",
				     __FILE__, __LINE__, id, id - V4L2_CID_BASE,
				     ret_get, errno_get, control_new.value);

				CU_ASSERT_EQUAL(ret_get, 0);
				if (ret_get == 0) {
					CU_ASSERT(queryctrl.minimum <=
						  control_new.value);
					CU_ASSERT(control_new.value <=
						  queryctrl.maximum);

					if (ret_set == 0) {
						/* TODO: the following checks works correctly only if
						 * S32_MIN <= queryctrl.minimum-queryctrl.step and
						 * queryctrl.maximum+queryctrl.step <= S32_MAX
						 */

/*
 * If we try to set the new control value to "value" then the possible results can be
 * "x" and "x-step". These two values can be expressed with the range
 * (value-step, value+step) where the ranges are not included.
 *
 *           value-step        value         value+step
 *              |                |                |
 *              |                v                |
 *              +----------------+----------------+
 *               *********************************
 * ... -+----------------+----------------+----------------+----------------+- ...
 *      |                |                |                |                |
 *   x-2*step          x-step             x              x+step          x+2*step
 *
 * The following figure shows the case when we try to set value to "x" which is
 * a possible set value. In this case the only valid result is the "x".
 *
 *                    value-step        value         value+step
 *                       |                |                |
 *                       |                v                |
 *                       +----------------+----------------+
 *                        *********************************
 * ... -+----------------+----------------+----------------+----------------+- ...
 *      |                |                |                |                |
 *   x-2*step          x-step             x              x+step          x+2*step
 *
 *
 *                                    value-step        value         value+step
 *                                       |                |                |
 *                                       |                v                |
 *                                       +----------------+----------------+
 *                                        *********************************
 * ... -+----------------+----------------+----------------+----------------+- ...
 *      |                |                |                |                |
 *   x-2*step          x-step             x              x+step          x+2*step
 *
 *
 */
						CU_ASSERT(value -
							  queryctrl.step <
							  control_new.value);
						CU_ASSERT(control_new.value <
							  value +
							  queryctrl.step);
					}
				}

			}

			break;

		case V4L2_CTRL_TYPE_BUTTON:
			/* This control only performs an action, does not have
			 * any value
			 */
			CU_ASSERT_EQUAL(ret_orig, -1);
			CU_ASSERT_EQUAL(errno_orig, EINVAL);

			/* The set value shall be ignored by button controls */

			memset(&control, 0xff, sizeof(control));
			control.id = id;
			control.value = S32_MIN;
			ret_set =
			    ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
			errno_set = errno;

			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
			    queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY) {
				CU_ASSERT_EQUAL(ret_set, -1);
				CU_ASSERT_EQUAL(errno_set, EINVAL);
			} else if (queryctrl.flags & V4L2_CTRL_FLAG_GRABBED) {
				CU_ASSERT_EQUAL(ret_set, -1);
				CU_ASSERT_EQUAL(errno_set, EBUSY);
			} else {
				CU_ASSERT_EQUAL(ret_set, 0);
			}

			memset(&control, 0xff, sizeof(control));
			control.id = id;
			control.value = -1;
			ret_set =
			    ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
			errno_set = errno;

			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
			    queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY) {
				CU_ASSERT_EQUAL(ret_set, -1);
				CU_ASSERT_EQUAL(errno_set, EINVAL);
			} else if (queryctrl.flags & V4L2_CTRL_FLAG_GRABBED) {
				CU_ASSERT_EQUAL(ret_set, -1);
				CU_ASSERT_EQUAL(errno_set, EBUSY);
			} else {
				CU_ASSERT_EQUAL(ret_set, 0);
			}

			memset(&control, 0xff, sizeof(control));
			control.id = id;
			control.value = 0;
			ret_set =
			    ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
			errno_set = errno;

			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
			    queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY) {
				CU_ASSERT_EQUAL(ret_set, -1);
				CU_ASSERT_EQUAL(errno_set, EINVAL);
			} else if (queryctrl.flags & V4L2_CTRL_FLAG_GRABBED) {
				CU_ASSERT_EQUAL(ret_set, -1);
				CU_ASSERT_EQUAL(errno_set, EBUSY);
			} else {
				CU_ASSERT_EQUAL(ret_set, 0);
			}

			memset(&control, 0xff, sizeof(control));
			control.id = id;
			control.value = 1;
			ret_set =
			    ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
			errno_set = errno;

			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
			    queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY) {
				CU_ASSERT_EQUAL(ret_set, -1);
				CU_ASSERT_EQUAL(errno_set, EINVAL);
			} else if (queryctrl.flags & V4L2_CTRL_FLAG_GRABBED) {
				CU_ASSERT_EQUAL(ret_set, -1);
				CU_ASSERT_EQUAL(errno_set, EBUSY);
			} else {
				CU_ASSERT_EQUAL(ret_set, 0);
			}

			memset(&control, 0xff, sizeof(control));
			control.id = id;
			control.value = S32_MAX;
			ret_set =
			    ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
			errno_set = errno;

			if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
			    queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY) {
				CU_ASSERT_EQUAL(ret_set, -1);
				CU_ASSERT_EQUAL(errno_set, EINVAL);
			} else if (queryctrl.flags & V4L2_CTRL_FLAG_GRABBED) {
				CU_ASSERT_EQUAL(ret_set, -1);
				CU_ASSERT_EQUAL(errno_set, EBUSY);
			} else {
				CU_ASSERT_EQUAL(ret_set, 0);
			}

			break;

		case V4L2_CTRL_TYPE_INTEGER64:	/* TODO: what about this case? */
		case V4L2_CTRL_TYPE_CTRL_CLASS:
		default:
			CU_ASSERT_EQUAL(ret_orig, -1);
			CU_ASSERT_EQUAL(errno_orig, -1);
		}
	} else {
		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);

		CU_ASSERT_EQUAL(ret_orig, -1);
		CU_ASSERT_EQUAL(errno_orig, EINVAL);

	}

	if (ret_orig == 0) {
		CU_ASSERT_EQUAL(ret_orig, 0);

		/* restore the original control value */
		value = control_orig.value;
		memset(&control, 0xff, sizeof(control));
		control.id = id;
		control.value = value;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
		errno_set = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_S_CTRL, id=%u (V4L2_CID_BASE+%i), value=%i, ret_set=%i, errno_set=%i\n",
		     __FILE__, __LINE__, id, id - V4L2_CID_BASE, value, ret_set,
		     errno_set);

		/* it shall be possible to set to the original value if the control
		 * is not disabled, read only or grabbed by other application
		 */
		if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
		    queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY) {
			CU_ASSERT_EQUAL(ret_set, -1);
			CU_ASSERT_EQUAL(errno_set, EINVAL);
		} else if (queryctrl.flags & V4L2_CTRL_FLAG_GRABBED) {
			CU_ASSERT_EQUAL(ret_set, -1);
			CU_ASSERT_EQUAL(errno_set, EBUSY);
		} else {
			CU_ASSERT_EQUAL(ret_set, 0);
		}

		memset(&control_new, 0, sizeof(control_new));
		control_new.id = id;
		ret_get = ioctl(get_video_fd(), VIDIOC_G_CTRL, &control_new);
		errno_get = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_G_CTRL, id=%u (V4L2_CID_BASE+%i), ret_get=%i, errno_get=%i, control_new.value=%i\n",
		     __FILE__, __LINE__, id, id - V4L2_CID_BASE, ret_get,
		     errno_get, control_new.value);

		CU_ASSERT_EQUAL(ret_get, 0);
		if (ret_get == 0) {
			CU_ASSERT(queryctrl.minimum <= control_new.value);
			CU_ASSERT(control_new.value <= queryctrl.maximum);
			CU_ASSERT_EQUAL(control_new.value, control_orig.value);
		}

	}

	return ret_query;
}

static void do_set_control_value(__u32 id, __s32 value,
				 struct v4l2_queryctrl *queryctrl);
static void do_set_control_value(__u32 id, __s32 value,
				 struct v4l2_queryctrl *queryctrl)
{
	int ret_set, errno_set;
	int ret_get, errno_get;
	struct v4l2_control control;
	struct v4l2_control control_new;

	memset(&control, 0xff, sizeof(control));
	control.id = id;
	control.value = value;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
	errno_set = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_S_CTRL, id=%u (V4L2_CID_BASE+%i), value=%i, ret_set=%i, errno_set=%i\n",
	     __FILE__, __LINE__, id, id - V4L2_CID_BASE, value, ret_set,
	     errno_set);

	/* The driver can decide if it returns ERANGE or
	 * accepts the value and converts it to
	 * the nearest limit
	 */
	if (ret_set == -1) {
		CU_ASSERT_EQUAL(ret_set, -1);
		if (!(queryctrl->flags & V4L2_CTRL_FLAG_DISABLED) &&
		    !(queryctrl->flags & V4L2_CTRL_FLAG_READ_ONLY)) {
			CU_ASSERT_EQUAL(errno_set, ERANGE);
		} else {
			CU_ASSERT_EQUAL(errno_set, EINVAL);
		}

		/* check whether the new value is not out of bounds */
		memset(&control_new, 0, sizeof(control_new));
		control_new.id = id;
		ret_get = ioctl(get_video_fd(), VIDIOC_G_CTRL, &control_new);
		errno_get = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_G_CTRL, id=%u (V4L2_CID_BASE+%i), ret_get=%i, errno_get=%i, control_new.value=%i\n",
		     __FILE__, __LINE__, id, id - V4L2_CID_BASE, ret_get,
		     errno_get, control_new.value);

		CU_ASSERT_EQUAL(ret_get, 0);
		if (ret_get == 0) {
			CU_ASSERT(queryctrl->minimum <= control_new.value);
			CU_ASSERT(control_new.value <= queryctrl->maximum);
		}

	} else {
		CU_ASSERT_EQUAL(ret_set, 0);

		/* check whether the new value is not out of bounds */
		memset(&control_new, 0, sizeof(control_new));
		control_new.id = id;
		ret_get = ioctl(get_video_fd(), VIDIOC_G_CTRL, &control_new);
		errno_get = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_G_CTRL, id=%u (V4L2_CID_BASE+%i), ret_get=%i, errno_get=%i, control_new.value=%i\n",
		     __FILE__, __LINE__, id, id - V4L2_CID_BASE, ret_get,
		     errno_get, control_new.value);

		CU_ASSERT_EQUAL(ret_get, 0);
		if (ret_get == 0) {
			CU_ASSERT(queryctrl->minimum <= control_new.value);
			CU_ASSERT(control_new.value <= queryctrl->maximum);
			CU_ASSERT_EQUAL(control_new.value, queryctrl->minimum);
		}
	}
}

int do_set_control_invalid(__u32 id)
{
	int ret_query, errno_query;
	int ret_set, errno_set;
	int ret_get, errno_get;
	int ret_orig, errno_orig;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_control control_orig;
	struct v4l2_control control;
	struct v4l2_control control_new;
	__s32 value;

	/* The expected return value of VIDIOC_S_CTRL depens on the value
	 * reported by VIDIOC_QUERYCTRL. The allowed limits are also
	 * reported by VIDIOC_QUERYCTRL.
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

	memset(&control_orig, 0, sizeof(control_orig));
	control_orig.id = id;
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_CTRL, &control_orig);
	errno_orig = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_G_CTRL, id=%u (V4L2_CID_BASE+%i), ret_orig=%i, errno_orig=%i, control_orig.value=%i\n",
	     __FILE__, __LINE__, id, id - V4L2_CID_BASE, ret_orig, errno_orig,
	     control_orig.value);

	if (ret_query == 0) {
		CU_ASSERT_EQUAL(ret_query, 0);

		switch (queryctrl.type) {
		case V4L2_CTRL_TYPE_INTEGER:
		case V4L2_CTRL_TYPE_BOOLEAN:
		case V4L2_CTRL_TYPE_MENU:
			if (S32_MIN < queryctrl.minimum) {
				do_set_control_value(id, S32_MIN, &queryctrl);
			}

			if (S32_MIN < queryctrl.minimum) {
				do_set_control_value(id, queryctrl.minimum - 1,
						     &queryctrl);
			}

			if (S16_MIN < queryctrl.minimum) {
				do_set_control_value(id, S16_MIN, &queryctrl);
			}

			if (U16_MIN < queryctrl.minimum) {
				do_set_control_value(id, U16_MIN, &queryctrl);
			}

			if (queryctrl.maximum < S16_MAX) {
				do_set_control_value(id, S16_MAX, &queryctrl);
			}

			if (queryctrl.maximum < (__s32) U16_MAX) {
				do_set_control_value(id, (__s32) U16_MAX,
						     &queryctrl);
			}

			if (queryctrl.maximum < S32_MAX) {
				do_set_control_value(id, queryctrl.maximum + 1,
						     &queryctrl);
			}

			if (queryctrl.maximum < S32_MAX) {
				do_set_control_value(id, S32_MAX, &queryctrl);
			}

			break;

		case V4L2_CTRL_TYPE_BUTTON:
			/* This control only performs an action, does not have
			 * any value
			 */
			CU_ASSERT_EQUAL(ret_orig, -1);
			CU_ASSERT_EQUAL(errno_orig, EINVAL);

			/* there is no invalid value for button control */

			break;

		case V4L2_CTRL_TYPE_INTEGER64:	/* TODO: what about this case? */
		case V4L2_CTRL_TYPE_CTRL_CLASS:
		default:
			CU_ASSERT_EQUAL(ret_orig, -1);
			CU_ASSERT_EQUAL(errno_orig, -1);
		}
	} else {
		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);

		CU_ASSERT_EQUAL(ret_orig, -1);
		CU_ASSERT_EQUAL(errno_orig, EINVAL);

		memset(&control, 0xff, sizeof(control));
		control.id = id;
		control.value = S32_MIN;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
		errno_set = errno;

		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

		memset(&control, 0xff, sizeof(control));
		control.id = id;
		control.value = -1;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
		errno_set = errno;

		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

		memset(&control, 0xff, sizeof(control));
		control.id = id;
		control.value = 0;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
		errno_set = errno;

		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

		memset(&control, 0xff, sizeof(control));
		control.id = id;
		control.value = 1;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
		errno_set = errno;

		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

		memset(&control, 0xff, sizeof(control));
		control.id = id;
		control.value = S32_MAX;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
		errno_set = errno;

		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

	}

	if (ret_orig == 0) {
		CU_ASSERT_EQUAL(ret_orig, 0);

		/* restore the original control value */
		value = control_orig.value;
		memset(&control, 0xff, sizeof(control));
		control.id = id;
		control.value = value;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_CTRL, &control);
		errno_set = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_S_CTRL, id=%u (V4L2_CID_BASE+%i), value=%i, ret_set=%i, errno_set=%i\n",
		     __FILE__, __LINE__, id, id - V4L2_CID_BASE, value, ret_set,
		     errno_set);

		/* it shall be possible to set to the original value if the control
		 * is not disabled, read only or grabbed by other application
		 */
		if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED ||
		    queryctrl.flags & V4L2_CTRL_FLAG_READ_ONLY) {
			CU_ASSERT_EQUAL(ret_set, -1);
			CU_ASSERT_EQUAL(errno_set, EINVAL);
		} else if (queryctrl.flags & V4L2_CTRL_FLAG_GRABBED) {
			CU_ASSERT_EQUAL(ret_set, -1);
			CU_ASSERT_EQUAL(errno_set, EBUSY);
		} else {
			CU_ASSERT_EQUAL(ret_set, 0);
		}

		memset(&control_new, 0, sizeof(control_new));
		control_new.id = id;
		ret_get = ioctl(get_video_fd(), VIDIOC_G_CTRL, &control_new);
		errno_get = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_G_CTRL, id=%u (V4L2_CID_BASE+%i), ret_get=%i, errno_get=%i, control_new.value=%i\n",
		     __FILE__, __LINE__, id, id - V4L2_CID_BASE, ret_get,
		     errno_get, control_new.value);

		CU_ASSERT_EQUAL(ret_get, 0);
		if (ret_get == 0) {
			CU_ASSERT(queryctrl.minimum <= control_new.value);
			CU_ASSERT(control_new.value <= queryctrl.maximum);
			CU_ASSERT_EQUAL(control_new.value, control_orig.value);
		}

	}

	return ret_query;
}

void test_VIDIOC_S_CTRL()
{
	int ret1;
	__u32 i;

	for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {
		if (i != V4L2_CID_AUTO_WHITE_BALANCE &&
		    i != V4L2_CID_DO_WHITE_BALANCE &&
		    i != V4L2_CID_RED_BALANCE &&
		    i != V4L2_CID_BLUE_BALANCE &&
		    i != V4L2_CID_AUTOGAIN && i != V4L2_CID_GAIN)
			ret1 = do_set_control(i);
	}

	ret1 = do_set_control(V4L2_CID_BASE - 1);
	ret1 = do_set_control(V4L2_CID_LASTP1);
	ret1 = do_set_control(V4L2_CID_PRIVATE_BASE - 1);

	i = V4L2_CID_PRIVATE_BASE;
	do {
		ret1 = do_set_control(i);
		i++;
	} while (ret1 == 0);

	ret1 = do_set_control(i);
}

void test_VIDIOC_S_CTRL_invalid()
{
	int ret1;
	__u32 i;

	for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {
		if (i != V4L2_CID_AUTO_WHITE_BALANCE &&
		    i != V4L2_CID_DO_WHITE_BALANCE &&
		    i != V4L2_CID_RED_BALANCE &&
		    i != V4L2_CID_BLUE_BALANCE &&
		    i != V4L2_CID_AUTOGAIN && i != V4L2_CID_GAIN)
			ret1 = do_set_control_invalid(i);
	}

	ret1 = do_set_control_invalid(V4L2_CID_BASE - 1);
	ret1 = do_set_control_invalid(V4L2_CID_LASTP1);
	ret1 = do_set_control_invalid(V4L2_CID_PRIVATE_BASE - 1);

	i = V4L2_CID_PRIVATE_BASE;
	do {
		ret1 = do_set_control_invalid(i);
		i++;
	} while (ret1 == 0);

	ret1 = do_set_control_invalid(i);
}

void test_VIDIOC_S_CTRL_white_balance()
{
	int ret1;

	/* TODO: handle V4L2_CID_AUTO_WHITE_BALANCE activated and deactivated separately */
	ret1 = do_set_control(V4L2_CID_AUTO_WHITE_BALANCE);
	ret1 = do_set_control(V4L2_CID_DO_WHITE_BALANCE);
	ret1 = do_set_control(V4L2_CID_RED_BALANCE);
	ret1 = do_set_control(V4L2_CID_BLUE_BALANCE);
}

void test_VIDIOC_S_CTRL_white_balance_invalid()
{
	int ret1;

	/* TODO: handle V4L2_CID_AUTO_WHITE_BALANCE activated and deactivated separately */
	ret1 = do_set_control_invalid(V4L2_CID_AUTO_WHITE_BALANCE);
	ret1 = do_set_control_invalid(V4L2_CID_DO_WHITE_BALANCE);
	ret1 = do_set_control_invalid(V4L2_CID_RED_BALANCE);
	ret1 = do_set_control_invalid(V4L2_CID_BLUE_BALANCE);
}

void test_VIDIOC_S_CTRL_gain()
{
	int ret1;

	/* TODO: handle V4L2_CID_AUTOGAIN activated and deactivated separately */
	ret1 = do_set_control(V4L2_CID_AUTOGAIN);
	ret1 = do_set_control(V4L2_CID_GAIN);
}

void test_VIDIOC_S_CTRL_gain_invalid()
{
	int ret1;

	/* TODO: handle V4L2_CID_AUTOGAIN activated and deactivated separately */
	ret1 = do_set_control_invalid(V4L2_CID_AUTOGAIN);
	ret1 = do_set_control_invalid(V4L2_CID_GAIN);
}

void test_VIDIOC_S_CTRL_NULL()
{
	int ret_null, errno_null;

	/* TODO: check whether VIDIOC_S_CTRL is supported or not */

	ret_null = ioctl(get_video_fd(), VIDIOC_S_CTRL, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_S_CTRL, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	CU_ASSERT_EQUAL(ret_null, -1);
	CU_ASSERT_EQUAL(errno_null, EFAULT);

}
