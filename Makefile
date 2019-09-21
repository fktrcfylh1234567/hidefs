CC=g++
CFLAGS=-Wall -ansi -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -DELPP_SYSLOG -DELPP_NO_DEFAULT_LOG_FILE -DELPP_THREAD_SAFE -std=c++11
LDFLAGS=-Wall -ansi -lfuse -lpthread
srcdir=src
builddir=build

all: $(builddir) hidefs

$(builddir):
	mkdir $(builddir)

hidefs: $(builddir)/hidefs.o
	$(CC) -o hidefs $(builddir)/hidefs.o $(LDFLAGS)

$(builddir)/hidefs.o: $(srcdir)/hidefs.cpp
	$(CC) -o $(builddir)/hidefs.o -c $(srcdir)/hidefs.cpp $(CFLAGS)

clean:
	rm -rf $(builddir)/
	rm hidefs
	
install:
	cp hidefs /usr/bin/
