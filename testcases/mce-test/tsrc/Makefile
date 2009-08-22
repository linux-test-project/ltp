LSRC := ../linux
BROWSER := firefox

CFLAGS += -I ${LSRC}/arch/x86/kernel/cpu/mcheck/ -g -Wall

KFLAGS := -I ./kinclude

all: standalone

standalone: tinjpage tinjpage-working tkillpoison

requireskernel: tcases ttable

tcases: tcases.c ${LSRC}/arch/x86/kernel/cpu/mcheck/mce-severity.c ${LSRC}/arch/x86/kernel/cpu/mcheck/mce-internal.h
	${CC} ${CFLAGS} ${KFLAGS} -o tcases tcases.c

ttable: ttable.c ${LSRC}/arch/x86/kernel/cpu/mcheck/mce-severity.c ${LSRC}/arch/x86/kernel/cpu/mcheck/mce-internal.h
	${CC} ${CFLAGS} ${KFLAGS} -o ttable ttable.c

tring: tring.o 

tring : LDFLAGS += -lpthread

x.html: ttable
	./ttable ${TFLAGS} > x.html

.PHONY: see

see: x.html
	${BROWSER} x.html

tinjpage: LDFLAGS += -lpthread

.PHONY: clean distclean

clean:
	rm -f tcases ttable x.html tinjpage	
	rm -f tkillpoison tring

distclean: clean


