
CFLAGS = -g

TARGET = libae.a

OBJS = ae.o

$(TARGET): $(OBJS)
	ar cr $@ $^

*.o*.c:
	gcc $(CFLAGS) -c $^ -o $@