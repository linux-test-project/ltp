include Makefile.inc
SUBDIRS=robust_test pi_test

.PHONY : all

all:
	$(foreach SUBDIR,$(SUBDIRS),make -C $(SUBDIR);)
install:
	$(foreach SUBDIR,$(SUBDIRS),make -C $(SUBDIR) install;)
clean:
	rm -f *~
	$(foreach SUBDIR,$(SUBDIRS),make -C $(SUBDIR) clean;)

