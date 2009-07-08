ALL_FILES = tomoyo_file_test newns

all: $(ALL_FILES)
	chmod 755 testall.sh

CC=gcc

CFLAGS=-Wall -O2 ${shell $(CC) -Wno-pointer-sign -S -o /dev/null -x c - < /dev/null > /dev/null 2>&1 && echo "-Wno-pointer-sign"}

#
# Tools for kernel testing.
#

.c:	include.h
	$(CC) $(CFLAGS) -o $@ $<
	chmod 700 $@

#
# Delete all tools in this package.
#

clean:
	rm -f $(ALL_FILES)
