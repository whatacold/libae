
CFLAGS = -g

DEMO = rot13d

LIB = libae.a

OBJS = ae.o

all: $(LIB) $(DEMO)

$(DEMO): rot13d.c $(LIB)
	gcc $(CFLAGS) -o $@ $^

$(LIB): $(OBJS)
	ar cr $@ $^

*.o*.c:
	gcc $(CFLAGS) -c $^ -o $@

clean:
	@rm -f *.o $(LIB) $(DEMO)