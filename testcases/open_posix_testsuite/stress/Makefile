SUBDIRS=	mqueues semaphores

all clean test:
	for d in $(SUBDIRS); do \
		$(MAKE) -C $$d $@; \
	done
