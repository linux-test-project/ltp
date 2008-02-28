TARGET=libcontrollers.a
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

all : $(TARGET)

$(TARGET): $(OBJS)
	$(AR) -cr $@ $^

clean:
	rm -f $(TARGET) $(OBJS)

install:


