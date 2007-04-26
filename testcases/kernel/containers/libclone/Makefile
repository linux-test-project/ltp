TARGET=libclone.a
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all noltp : $(TARGET)

$(TARGET): $(OBJS)
	$(AR) -cr $@ $^

clean:
	rm -f $(TARGET) $(OBJS)

noltp_check: noltp

install:
