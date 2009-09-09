CFLAGS=-Wall -Werror -g -I.. -g
LDFLAGS=-L.. -g -lgenhash -lm
THELIB=../libgenhash.a

PROGS=hashtest
OBJS=hashtest.o

all: $(PROGS)

hashtest: hashtest.o
	$(CC) -o $@ hashtest.o $(LDFLAGS)

$(OBJS): $(THELIB)

clean:
	rm $(PROGS) $(OBJS)
