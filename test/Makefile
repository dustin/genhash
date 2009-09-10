CFLAGS=-Wall -Werror -g -I.. -g
LDFLAGS=-L.. -g -lgenhash -lm
THELIB=../libgenhash.a

.PHONY: test

PROGS=hashtest
OBJS=hashtest.o

all: test

test: hashtest
	./hashtest

hashtest: hashtest.o
	$(CC) -o $@ hashtest.o $(LDFLAGS)

$(OBJS): $(THELIB)

clean:
	rm $(PROGS) $(OBJS)
