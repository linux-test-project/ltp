CFLAGS+=	-I../include
LDLIBS=		-lpthread
TARGETS=	pitest-1 pitest-2 pitest-3 pitest-4 pitest-5 pitest-6

all: $(TARGETS)

clean:
	-rm -rf $(TARGETS)

test:
	./run.sh
