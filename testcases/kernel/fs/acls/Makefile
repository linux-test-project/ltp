check_header = $(shell \
	if echo "\#include <$(1)>" | $(CC) -E - > /dev/null 2>&1 ; \
	then echo yes ; \
	else echo no ; fi)

ifeq ($(call check_header,sys/xattr.h),yes)

all: acl_file_test acl_link_test

acl_file_test: acl_file_test.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o acl_file_test acl_file_test.c
acl_link_test: acl_link_test.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o acl_link_test acl_link_test.c

install:
	ln -f acl_file_test ../../../bin/
	ln -f acl_link_test ../../../bin/
	ln -f acl_test01 ../../../bin/

else

all install:
	@echo File sys/xattr.h does not exist, attr tests cannot be build!
endif

clean:
	rm -f acl_file_test acl_link_test
