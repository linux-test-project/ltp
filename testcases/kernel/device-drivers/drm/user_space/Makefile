
	KDIR := /lib/modules/$(shell uname -r)/build
	CFLAGS := -I$(KDIR)/drivers/char/drm
default:
	${CC} -o test_drm  user_tdrm.c $(CFLAGS)
