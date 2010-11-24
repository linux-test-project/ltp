LSRC := ../linux
BROWSER := firefox

CFLAGS += -I ${LSRC}/arch/x86/kernel/cpu/mcheck/ -g -Wall

KFLAGS := -I ./kinclude

EXE := tinjpage tsimpleinj tkillpoison tprctl tsoft tsoftinj thugetlb
EXEKERNEL := tring ttable

OBJ := $(addsuffix .o,${EXE})
OBJKERNEL := $(addsuffix .o,${EXEKERNEL})

.PHONY: clean distclean see test hard soft standalone

all: standalone

standalone: ${EXE}

kernel: ${EXEKERNEL}

tcases: tcases.c ${LSRC}/arch/x86/kernel/cpu/mcheck/mce-severity.c ${LSRC}/arch/x86/kernel/cpu/mcheck/mce-internal.h
	${CC} ${CFLAGS} ${KFLAGS} -o tcases tcases.c

ttable: ttable.c ${LSRC}/arch/x86/kernel/cpu/mcheck/mce-severity.c ${LSRC}/arch/x86/kernel/cpu/mcheck/mce-internal.h
	${CC} ${CFLAGS} ${KFLAGS} -o ttable ttable.c

tprctl: tprctl.o

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
	rm -f ${EXE} ${OBJ}
	rm -f ${EXEKERNEL} ${OBJKERNEL}
	rm -f x.html unpoison-failed offlined
	rm -f ~test* ~poison*

distclean: clean

test: soft hard

hard: standalone
	./tinjpage
	./tsimpleinj
	if ! ./tkillpoison ; then echo "killed as expected" ; exit 0 ; else echo "didn't get killed" ; exit 1 ; fi
	./tprctl

soft: standalone
	./tsoft
	./tsoftinj
	echo "Running soft offliner for 60 seconds"
	./random_offline -t 60

test-kernel: tcases
	./tcases



