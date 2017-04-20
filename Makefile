CC=g++
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS=-I. -g -Wall
DEPS = tcmalloc-test.h

LIBS=-ltcmalloc -lncurses -lpthread -lm

#%.o: $(DEPS)
#	$(CC) -c $(LIBS) -o $@ %.cc $(CFLAGS)

tcmalloc-test: clean tcmalloc-allocator.o tcmalloc-thread.o tcmalloc-menu.o tcmalloc-test.o
	$(CC) -std=c++11 -o tcmalloc-test tcmalloc-test.o tcmalloc-allocator.o tcmalloc-menu.o tcmalloc-thread.o -I. $(LIBS)

tcmalloc-test.o:
	$(CC) -std=c++11 -g -c $(LIBS) -o tcmalloc-test.o tcmalloc-test.cc

tcmalloc-allocator.o:
	$(CC) -std=c++11 -g -c $(LIBS) -o tcmalloc-allocator.o tcmalloc-allocator.cc

tcmalloc-thread.o:
	$(CC) -std=c++11 -g -c $(LIBS) -o tcmalloc-thread.o tcmalloc-thread.cc

tcmalloc-menu.o:
	$(CC) -std=c++11 -g -c $(LIBS) -o tcmalloc-menu.o tcmalloc-menu.cc

.PHONY: clean

clean:
	rm -f *.o tcmalloc-test
