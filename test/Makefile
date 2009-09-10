CFLAGS=-Wall -Werror -g -I.. -g
LDFLAGS=-L.. -g -lgenhash -lm
THELIB=../libgenhash.a

.PHONY: test

PROGS=hashtest sizingtest
OBJS=hashtest.o sizingtest.o

all: test

test: hashtest sizingtest
	./hashtest
	./sizingtest

hashtest: hashtest.o
	$(CC) -o $@ hashtest.o $(LDFLAGS)

sizingtest: sizingtest.o
	$(CC) -o $@ sizingtest.o $(LDFLAGS)

$(OBJS): $(THELIB)

clean:
	rm $(PROGS) $(OBJS)
