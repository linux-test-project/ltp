/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  2 Jan 2009  0.1  First release
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

#include "test_VIDIOC_QUERYCTRL.h"

static int valid_control_flag(__u32 flags) {
	int valid = 0;

	if ( (flags & ~(V4L2_CTRL_FLAG_DISABLED |
			V4L2_CTRL_FLAG_GRABBED |
			V4L2_CTRL_FLAG_READ_ONLY |
			V4L2_CTRL_FLAG_UPDATE |
			V4L2_CTRL_FLAG_INACTIVE |
			V4L2_CTRL_FLAG_SLIDER))
		== 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

static int valid_control_type(__u32 type) {
	int valid = 0;

	switch (type) {
	case V4L2_CTRL_TYPE_INTEGER:
	case V4L2_CTRL_TYPE_BOOLEAN:
	case V4L2_CTRL_TYPE_MENU:
	case V4L2_CTRL_TYPE_BUTTON:
	case V4L2_CTRL_TYPE_INTEGER64:
	case V4L2_CTRL_TYPE_CTRL_CLASS:
		valid = 1;
		break;
	default:
		valid = 0;
	}
	return valid;
}

void test_VIDIOC_QUERYCTRL() {
	int ret;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;
	__u32 i;

	for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {

		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);

		dprintf("VIDIOC_QUERYCTRL, id=%u (V4L2_CID_BASE+%i), ret=%i\n",
			i, i-V4L2_CID_BASE, ret);

		if (ret == 0) {
			CU_ASSERT_EQUAL(ret, 0);
			CU_ASSERT_EQUAL(queryctrl.id, i);

			//CU_ASSERT_EQUAL(queryctrl.name, ?);
			CU_ASSERT(0 < strlen( (char*)queryctrl.name ));

			CU_ASSERT(valid_control_type(queryctrl.type));

			switch (queryctrl.type) {
			case V4L2_CTRL_TYPE_INTEGER:
				/* min < max, because otherwise this control makes no sense */
				CU_ASSERT(queryctrl.minimum < queryctrl.maximum);

				CU_ASSERT(0 < queryctrl.step);

				CU_ASSERT(queryctrl.minimum <= queryctrl.default_value);
				CU_ASSERT(queryctrl.default_value <= queryctrl.maximum);
				break;

			case V4L2_CTRL_TYPE_BOOLEAN:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 1);
				CU_ASSERT_EQUAL(queryctrl.step, 1);
				CU_ASSERT((queryctrl.default_value == 0) || (queryctrl.default_value == 1));
				break;

			case V4L2_CTRL_TYPE_MENU:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT(queryctrl.minimum <= queryctrl.default_value);
				CU_ASSERT_EQUAL(queryctrl.step, 1);
				CU_ASSERT(queryctrl.minimum <= queryctrl.default_value);
				CU_ASSERT(queryctrl.default_value <= queryctrl.maximum);
				break;

			case V4L2_CTRL_TYPE_BUTTON:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
				break;

			case V4L2_CTRL_TYPE_INTEGER64: /* fallthrough */
			case V4L2_CTRL_TYPE_CTRL_CLASS:
				/* These parameters are defined as n/a by V4L2, so
				 * they should be filled with zeros, the same like
				 * the reserved fields.
				 */ 
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
				break;

			default:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
			}

			CU_ASSERT(valid_control_flag(queryctrl.flags));

			CU_ASSERT_EQUAL(queryctrl.reserved[0], 0);
			CU_ASSERT_EQUAL(queryctrl.reserved[1], 0);

			dprintf("\tqueryctrl = {.id=%u, .type=%i, .name=\"%s\", "
				".minimum=%i, .maximum=%i, .step=%i, "
				".default_value=%i, "
				".flags=0x%X, "
				".reserved[]={ 0x%X, 0x%X } }\n",
				queryctrl.id,
				queryctrl.type,
				queryctrl.name,
				queryctrl.minimum,
				queryctrl.maximum,
				queryctrl.step,
				queryctrl.default_value,
				queryctrl.flags,
				queryctrl.reserved[0],
				queryctrl.reserved[1]
				);

		} else {
			CU_ASSERT_EQUAL(ret, -1);
			CU_ASSERT_EQUAL(errno, EINVAL);

			memset(&queryctrl2, 0xff, sizeof(queryctrl2));
			queryctrl2.id = i;
			CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

			dprintf("\terrno=%i\n", errno);

		}
	}

}

void test_VIDIOC_QUERYCTRL_BASE_1() {
	int ret;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;

	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = V4L2_CID_BASE-1;
	ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);

	dprintf("VIDIOC_QUERYCTRL, id=%u (V4L2_CID_BASE-1), ret=%i\n",
			V4L2_CID_BASE-1, ret);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&queryctrl2, 0xff, sizeof(queryctrl2));
	queryctrl2.id = V4L2_CID_BASE-1;
	CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

}

void test_VIDIOC_QUERYCTRL_LASTP1() {
	int ret;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;

	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = V4L2_CID_LASTP1;
	ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);

	dprintf("VIDIOC_QUERYCTRL, id=%u (V4L2_CID_LASTP1), ret=%i\n",
			V4L2_CID_LASTP1, ret);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&queryctrl2, 0xff, sizeof(queryctrl2));
	queryctrl2.id = V4L2_CID_LASTP1;
	CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

}

void test_VIDIOC_QUERYCTRL_LASTP1_1() {
	int ret;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;

	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = V4L2_CID_LASTP1+1;
	ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);

	dprintf("VIDIOC_QUERYCTRL, id=%u (V4L2_CID_LASTP1+1), ret=%i\n",
			V4L2_CID_LASTP1+1, ret);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&queryctrl2, 0xff, sizeof(queryctrl2));
	queryctrl2.id = V4L2_CID_LASTP1+1;
	CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

}


void test_VIDIOC_QUERYCTRL_flag_NEXT_CTRL() {
	int ret;
	char count_controls1[V4L2_CID_LASTP1-V4L2_CID_BASE];
	char count_controls2[V4L2_CID_LASTP1-V4L2_CID_BASE];
	struct v4l2_queryctrl controls[V4L2_CID_LASTP1-V4L2_CID_BASE];
	struct v4l2_queryctrl queryctrl;
	__u32 i;

	/* find out all the possible user controls */
	memset(count_controls1, 0, sizeof(count_controls1));
	memset(controls, 0, sizeof(controls));

	for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {

		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);

		if (ret == 0) {
			CU_ASSERT_EQUAL(ret, 0);
			CU_ASSERT_EQUAL(queryctrl.id, i);
			count_controls1[i-V4L2_CID_BASE]++;
			controls[i-V4L2_CID_BASE] = queryctrl;

			dprintf("\tqueryctrl = {.id=%u, .type=%i, .name=\"%s\", "
				".minimum=%i, .maximum=%i, .step=%i, "
				".default_value=%i, "
				".flags=0x%X, "
				".reserved[]={ 0x%X, 0x%X } }\n",
				queryctrl.id,
				queryctrl.type,
				queryctrl.name,
				queryctrl.minimum,
				queryctrl.maximum,
				queryctrl.step,
				queryctrl.default_value,
				queryctrl.flags,
				queryctrl.reserved[0],
				queryctrl.reserved[1]
				);

		} else {
			CU_ASSERT_EQUAL(ret, -1);
			CU_ASSERT_EQUAL(errno, EINVAL);
		}
	}

	/* enumerate the controls with V4L2_CTRL_FLAG_NEXT_CTRL */
	dprintf1("Starting enumeration with V4L2_CTRL_FLAG_NEXT_CTRL\n");
	memset(count_controls2, 0, sizeof(count_controls2));

	/* As described at V4L2 Chapter 1.9.3. Enumerating Extended Controls */
	i = 0;
	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = i | V4L2_CTRL_FLAG_NEXT_CTRL;
	dprintf("\tasking for id=%i=V4L2_CID_BASE+%i | V4L2_CTRL_FLAG_NEXT_CTRL\n", i, i-V4L2_CID_BASE);
	ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
	dprintf("\tret=%i\n", ret);

	if (ret == 0) {
		do {
			/* protect the count_controls2[] from overindexing */
			if ((V4L2_CID_BASE <= queryctrl.id) && (queryctrl.id < V4L2_CID_LASTP1)) {
				count_controls2[queryctrl.id-V4L2_CID_BASE]++;
				CU_ASSERT_EQUAL(memcmp(&queryctrl, &controls[queryctrl.id-V4L2_CID_BASE], sizeof(queryctrl)), 0);
			}

			/* "The VIDIOC_QUERYCTRL ioctl will return the first 
			 *  control with a higher ID than the specified one."
			 */
			CU_ASSERT(i < queryctrl.id);

			CU_ASSERT(V4L2_CID_BASE <= queryctrl.id);
			CU_ASSERT(queryctrl.id < V4L2_CID_LASTP1);

			dprintf("\tqueryctrl = {.id=%u, .type=%i, .name=\"%s\", "
				".minimum=%i, .maximum=%i, .step=%i, "
				".default_value=%i, "
				".flags=0x%X, "
				".reserved[]={ 0x%X, 0x%X } }\n",
				queryctrl.id,
				queryctrl.type,
				queryctrl.name,
				queryctrl.minimum,
				queryctrl.maximum,
				queryctrl.step,
				queryctrl.default_value,
				queryctrl.flags,
				queryctrl.reserved[0],
				queryctrl.reserved[1]
				);

			i = queryctrl.id;
			memset(&queryctrl, 0xff, sizeof(queryctrl));
			queryctrl.id = i | V4L2_CTRL_FLAG_NEXT_CTRL;
			dprintf("\tasking for id=%i=V4L2_CID_BASE+%i | V4L2_CTRL_FLAG_NEXT_CTRL\n", i, i-V4L2_CID_BASE);
			ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
			dprintf("\tret=%i\n", ret);

		} while (ret == 0 && V4L2_CTRL_ID2CLASS(queryctrl.id) == V4L2_CTRL_CLASS_USER);

		if (ret == 0) {
			/* some other controls also exists, stop for now. */
		} else {
			CU_ASSERT_EQUAL(ret, -1);
			CU_ASSERT_EQUAL(errno, EINVAL);
		}

		/* Check whether the same controls are reported if using 
		 * V4L2_CTRL_FLAG_NEXT_CTRL and without using it.
		 * This also checks if one control is not reported twice.
		 */
		CU_ASSERT_EQUAL(memcmp(count_controls1, count_controls2, sizeof(count_controls1)), 0);
		
		dprintf1("count_controls1 = { ");
		for (i=0; i<sizeof(count_controls1)/sizeof(*count_controls1); i++) {
		    dprintf("%i ", count_controls1[i]);
		}
		dprintf1("}\n");

		dprintf1("count_controls2 = { ");
		for (i=0; i<sizeof(count_controls2)/sizeof(*count_controls2); i++) {
		    dprintf("%i ", count_controls2[i]);
		}
		dprintf1("}\n");
	
	} else {
		dprintf1("V4L2_CTRL_FLAG_NEXT_CTRL is not supported or no control is available\n");
		/* The flag V4L2_CTRL_FLAG_NEXT_CTRL is not supported
		 * or no control is avaliable at all. Do not continue the
		 * enumeration.
		 */
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);
	}

}



void test_VIDIOC_QUERYCTRL_private() {
	int ret;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;
	__u32 i;

	i = V4L2_CID_PRIVATE_BASE;
	do {
		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);

		dprintf("VIDIOC_QUERYCTRL, id=%u (V4L2_CID_BASE+%i), ret=%i\n",
			i, i-V4L2_CID_BASE, ret);

		if (ret == 0) {
			CU_ASSERT_EQUAL(ret, 0);
			CU_ASSERT_EQUAL(queryctrl.id, i);

			//CU_ASSERT_EQUAL(queryctrl.name, ?);
			CU_ASSERT(0 < strlen( (char*)queryctrl.name ));

			CU_ASSERT(valid_control_type(queryctrl.type));

			switch (queryctrl.type) {
			case V4L2_CTRL_TYPE_INTEGER:
				/* min < max, because otherwise this control makes no sense */
				CU_ASSERT(queryctrl.minimum < queryctrl.maximum);

				CU_ASSERT(0 < queryctrl.step);

				CU_ASSERT(queryctrl.minimum <= queryctrl.default_value);
				CU_ASSERT(queryctrl.default_value <= queryctrl.maximum);
				break;

			case V4L2_CTRL_TYPE_BOOLEAN:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 1);
				CU_ASSERT_EQUAL(queryctrl.step, 1);
				CU_ASSERT((queryctrl.default_value == 0) || (queryctrl.default_value == 1));
				break;

			case V4L2_CTRL_TYPE_MENU:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT(queryctrl.minimum <= queryctrl.default_value);
				CU_ASSERT_EQUAL(queryctrl.step, 1);
				CU_ASSERT(queryctrl.minimum <= queryctrl.default_value);
				CU_ASSERT(queryctrl.default_value <= queryctrl.maximum);
				break;

			case V4L2_CTRL_TYPE_BUTTON:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
				break;

			case V4L2_CTRL_TYPE_INTEGER64: /* fallthrough */
			case V4L2_CTRL_TYPE_CTRL_CLASS:
				/* These parameters are defined as n/a by V4L2, so
				 * they should be filled with zeros, the same like
				 * the reserved fields.
				 */ 
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
				break;

			default:
				CU_ASSERT_EQUAL(queryctrl.minimum, 0);
				CU_ASSERT_EQUAL(queryctrl.maximum, 0);
				CU_ASSERT_EQUAL(queryctrl.step, 0);
				CU_ASSERT_EQUAL(queryctrl.default_value, 0);
			}

			CU_ASSERT(valid_control_flag(queryctrl.flags));

			CU_ASSERT_EQUAL(queryctrl.reserved[0], 0);
			CU_ASSERT_EQUAL(queryctrl.reserved[1], 0);

			dprintf("\tqueryctrl = {.id=%u, .type=%i, .name=\"%s\", "
				".minimum=%i, .maximum=%i, .step=%i, "
				".default_value=%i, "
				".flags=0x%X, "
				".reserved[]={ 0x%X, 0x%X } }\n",
				queryctrl.id,
				queryctrl.type,
				queryctrl.name,
				queryctrl.minimum,
				queryctrl.maximum,
				queryctrl.step,
				queryctrl.default_value,
				queryctrl.flags,
				queryctrl.reserved[0],
				queryctrl.reserved[1]
				);

		} else {
			CU_ASSERT_EQUAL(ret, -1);
			CU_ASSERT_EQUAL(errno, EINVAL);

			memset(&queryctrl2, 0xff, sizeof(queryctrl2));
			queryctrl2.id = i;
			CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

			dprintf("\terrno=%i\n", errno);

		}
	} while (ret == 0);

}

void test_VIDIOC_QUERYCTRL_private_base_1() {
	int ret;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;

	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = V4L2_CID_PRIVATE_BASE-1;
	ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);

	dprintf("VIDIOC_QUERYCTRL, id=%u (V4L2_CID_PRIVATE_BASE-1), ret=%i\n",
			V4L2_CID_PRIVATE_BASE-1, ret);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&queryctrl2, 0xff, sizeof(queryctrl2));
	queryctrl2.id = V4L2_CID_PRIVATE_BASE-1;
	CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

}

void test_VIDIOC_QUERYCTRL_private_last_1() {
	int ret;
	struct v4l2_queryctrl queryctrl;
	struct v4l2_queryctrl queryctrl2;
	__u32 i;

	i = V4L2_CID_PRIVATE_BASE;
	do {
		memset(&queryctrl, 0xff, sizeof(queryctrl));
		queryctrl.id = i;
		ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);
		
		i++;
	} while (ret == 0);

	memset(&queryctrl, 0xff, sizeof(queryctrl));
	queryctrl.id = i;
	ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, &queryctrl);

	dprintf("VIDIOC_QUERYCTRL, id=%u (V4L2_CID_PRIVATE_BASE+%u), ret=%i\n",
			i, i-V4L2_CID_PRIVATE_BASE, ret);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&queryctrl2, 0xff, sizeof(queryctrl2));
	queryctrl2.id = i;
	CU_ASSERT_EQUAL(memcmp(&queryctrl, &queryctrl2, sizeof(queryctrl)), 0);

}


void test_VIDIOC_QUERYCTRL_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_QUERYCTRL, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}
