#
# v4l-test: Test environment for Video For Linux Two API
#
# 22 Dec 2008  0.2  test_VIDIOC_QUERYCAP.c added
# 18 Dec 2008  0.1  First release
#
# Written by Márton Németh <nm127@freemail.hu>
# Released under GPL
#

CFLAGS = -O2 -Wall -Wextra
#CFLAGS += -g
LDFLAGS = -lcunit

all:
	gcc $(CFLAGS) $(LDFLAGS) \
		dev_video.c \
		test_VIDIOC_QUERYCAP.c \
		test_VIDIOC_CROPCAP.c \
		test_VIDIOC_STD.c \
		test_VIDIOC_INPUT.c \
		test_invalid_ioctl.c \
		test_VIDIOC_ENUM_FMT.c \
		test_VIDIOC_ENUMINPUT.c \
		test_VIDIOC_ENUMSTD.c \
		test_VIDIOC_ENUMAUDIO.c \
		v4l2_test.c \
		-o v4l-test

clean:
	rm -f v4l-test
