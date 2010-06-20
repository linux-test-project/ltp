SUBDIRS=	robust_test pi_test

all install:
	for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@; \
	done

clean:
	rm -f *~
	for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@; \
	done
