
CFLAGS+= -Wall -I../include 
LDFLAGS+=
TARGET=libltp.a
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

$(TARGET): $(OBJS)
	$(AR) -cr $@ $^    

clean:
	rm -f $(TARGET) $(OBJS)
