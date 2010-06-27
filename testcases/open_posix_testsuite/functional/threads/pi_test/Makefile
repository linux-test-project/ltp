CFLAGS+=	-I../include
LDLIBS=		-lpthread
TARGETS=	pitest-1 pitest-2 pitest-3 pitest-4 pitest-5 pitest-6

all install: $(TARGETS)

clean:
	-rm -rf $(TARGETS)

test:
	./run.sh
all clean install test:
	for dir in `ls -d */Makefile 2>/dev/null | sed -e 's,/Makefile$$,,g'`; do \
		$(MAKE) -C $$dir $@;						  \
	done
