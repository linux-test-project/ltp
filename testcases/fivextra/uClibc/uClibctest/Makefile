

CROSS=i386-linux-
CC= $(CROSS)gcc
STRIPTOOL=strip
CFLAGS=-Wall -Wstrict-prototypes -O0 

EXTRA_LIBS=-lcrypt

HOSTCC=gcc
HOSTCFLAGS=-O2 -Wall



UCLIBC_TARGETS = crypt md5c-test ctype sem dirent mmap test_pwd test_grp pwcat grcat getgroups setjmp_test signal sigchld hello  \
tiny stat teststrtol qsort teston_exit testatexit ptytest \

GLIBC_TARGETS = crypt_glibc dirent_glibc test_pwd_glibc test_grp_glibc pwcat_glibc grcat_glibc getgroups_glibc stat_glibc \
teststrtol_glibc qsort_glibc teston_exit_glibc testatexit_glibc

TESTDIR = uClibctest

all:	$(UCLIBC_TARGETS) $(GLIBC_TARGETS) 

$(UCLIBC_TARGETS): $(patsubst %,%.c,$(UCLIBC_TARGETS))
	$(CC)  $(CFLAGS)  $(patsubst %,%.c,$@) -o $@ $(EXTRA_LIBS) 

$(GLIBC_TARGETS): $(patsubst %_glibc,%.c,$(GLIBC_TARGETS))
	$(HOSTCC) $(GLIBC_LDFLAGS)  $(patsubst %_glibc,%.c,$@) -o $@ $(EXTRA_LIBS) 

install:	
	mkdir -p ../../../bin/$(TESTDIR)
	@for f in $(UCLIBC_TARGETS) $(GLIBC_TARGETS) dirent.c stat.c crypt.input ; do cp $$f ../../../bin/$(TESTDIR)/$$f || break ; done

uninstall:
	@for f in $(UCLIBC_TARGETS) $(GLIBC_TARGETS) dirent.c stat.c crypt.input ; do rm -f ../../../bin/$(TESTDIR)/$$f || break ; done
	@rm -rf ../../../bin/$(TESTDIR)

clean distclean:
	@rm -f $(UCLIBC_TARGETS) $(GLIBC_TARGETS) *.o core 


