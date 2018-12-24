APPNAME=hellod
VERSION=1.0

# Where to install.
PREFIX=/usr/local

# What OS?
UNAME:=$(shell uname)

# Special compiler?
ifeq ($(UNAME), Linux)
CC=gcc
endif
ifeq ($(UNAME), Darwin)
CC=gcc
endif
ifeq ($(UNAME), FreeBSD)
CC=clang
endif

# Basic options.
#FIXME - remove debugging options
CFLAGS=-g -DDEBUG -std=c99 -W -Wall -DAPPNAME=$(APPNAME) -DVERSION=$(VERSION)
LFLAGS=-g
LIBS=-lconfuse

# The other bits.
DOCS = README License ChangeLog ${APPNAME}.8
# And all the code.
CODE = Makefile $(wildcard *.h) $(wildcard *.c)

# All the C source files.
SOURCES = $(wildcard *.c)
# All compiled C source files.
OBJECTS = $(SOURCES:.c=.o)

# Compile one file
.c.o:
	$(CC) $(CFLAGS) -MMD -c $<

# Default target.
all: $(APPNAME)

# Pull in header info.
-include *.d

# Link the app.
$(APPNAME): $(OBJECTS)
	$(CC) $(LFLAGS) -o $(APPNAME) $(OBJECTS) $(LIBS)

# Manpage.
$(APPNAME).conf.5.gz: $(APPNAME).conf.5
	cat $(APPNAME).conf.5 | gzip -9 > $(APPNAME).conf.5.gz
$(APPNAME).8.gz: $(APPNAME).8
	cat $(APPNAME).8 | gzip -9 > $(APPNAME).8.gz

# Install the app.
install: $(APPNAME) $(APPNAME).conf.5.gz $(APPNAME).8.gz
	install -D -m755 $(APPNAME) $(PREFIX)/sbin/$(APPNAME)
	install -D -m644 $(APPNAME).conf.5.gz $(PREFIX)/man/man5/$(APPNAME).conf.5.gz
	install -D -m644 $(APPNAME).8.gz $(PREFIX)/man/man8/$(APPNAME).8.gz
	install -D -m644 $(APPNAME).conf /etc/$(APPNAME).conf
	install -D -m644 $(APPNAME).service /etc/systemd/system/$(APPNAME).service
#	install -D -m755 $(APPNAME).init.sh /etc/init.d/$(APPNAME)
uninstall:
	rm -f $(PREFIX)/sbin/$(APPNAME)
	rm -f $(PREFIX)/man/man5/$(APPNAME).conf.5.gz
	rm -f $(PREFIX)/man/man8/$(APPNAME).8.gz
	rm -f /etc/$(APPNAME).conf
	rm -f /etc/systemd/system/$(APPNAME).service
#	rm -f /etc/init.d/$(APPNAME)

# Zap the cruft.
clean:
	rm -f *~
	rm -f *.d
	rm -f *.o
	rm -f $(APPNAME)
	rm -f $(APPNAME).conf.5.gz
	rm -f $(APPNAME).8.gz
	rm -rf $(APPNAME).dSYM
distclean: clean
	rm -f tags
	rm -f ${APPNAME}-${VERSION}.zip
	rm -f ${APPNAME}-${VERSION}.tar.gz

# Generate an archive.
zip:
	zip -9 ${APPNAME}-${VERSION}.zip ${CODE} ${DOCS} &> /dev/null
tar:
	tar --transform "s#^#${APPNAME}-${VERSION}/#" -zcvf ${APPNAME}-${VERSION}.tar.gz ${CODE} ${DOCS} &> /dev/null


# vim:ts=2:sw=2:tw=114:fo=tcnq2b:foldmethod=indent
