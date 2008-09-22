check_header = $(shell \
	if echo "\#include <$(1)>" | $(CC) -E - > /dev/null 2>&1 ; \
	then echo yes ; \
	else echo no ; fi)

ifeq ($(call check_header,sys/xattr.h),yes)
TARGETS := acl_file_test acl_link_test
else
TARGETS :=
endif

all: $(TARGETS)

acl_file_test: acl_file_test.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o acl_file_test acl_file_test.c
acl_link_test: acl_link_test.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o acl_link_test acl_link_test.c

install:
	chmod 755 acl_test01
	@set -e; for i in $(TARGETS) acl_test01 ; do ln -f $$i ../../../bin/ ; done

clean:
	rm -f acl_file_test acl_link_test
