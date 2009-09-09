CFLAGS=-Wall -Wextra -Werror -g

OBJS=genhash.o

THELIB=libgenhash.a

all: $(THELIB)

clean:
	rm $(OBJS) $(THELIB)

.PHONY: docs
docs:
	(cat Doxyfile ; echo "PROJECT_NUMBER=`git describe`") | doxygen -

genhash.o: genhash.c genhash.h

$(THELIB): $(OBJS)
	$(AR) rcs $@ $(OBJS)
	ranlib $@
