CFLAGS+=	-Wall
CPPFLAGS+=	-I../../include
LDLIBS=		-lpthread -lrt

all: send_rev_1.test send_rev_2.test 

%.test: %.o
	$(LINK.o) $< $(LOADLIBES) $(LDLIBS) -o $@

clean: 
	rm -f *.test
