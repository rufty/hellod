APPNAME=hellod
VERSION=1.1

# Where to install.
PREFIX=/usr/local

# Special compiler?
CC=clang

# Common options.
CFLAGS=-std=c17 -W -Wall -DAPPNAME=$(APPNAME) -DVERSION=$(VERSION)
LFLAGS=

#FIXME - remove debugging options
CFLAGS+=-g -DDEBUG
LFLAGS+=-g

# Libraries, all versions.
LIBS=-lconfuse

# What OS?
UNAME=$(shell uname)
ifeq (Linux,$(UNAME))
endif
ifeq (Darwin,$(UNAME))
endif
ifeq (FreeBSD,$(UNAME))
LFLAGS+=-L/usr/local/lib
CFLAGS+=-I/usr/local/include
endif
ifeq (MINGW32,$(findstring MINGW32, $(UNAME)))
LFLAGS+=-static
CFLAGS+=-I/mingw32/include
LIBS+=-liconv
endif
ifeq (MINGW64,$(findstring MINGW64, $(UNAME)))
LFLAGS+=-static
CFLAGS+=-I/mingw64/include
LIBS+=-liconv
endif

# The other bits.
DOCS = README LICENSE INSTALL ChangeLog
# All the code.
CODE = Makefile $(wildcard *.h) $(wildcard *.c)
# Other bits.
MISC = .gitignore .ycm_extra_conf.py
MISC+= $(APPNAME).8 $(APPNAME).conf $(APPNAME).conf.5
MISC+= $(APPNAME).service $(APPNAME).init.sh $(APPNAME).freebsd

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
$(APPNAME).8.gz: $(APPNAME).8
	cat $(APPNAME).8 | gzip -9 > $(APPNAME).8.gz
$(APPNAME).conf.5.gz: $(APPNAME).conf.5
	cat $(APPNAME).conf.5 | gzip -9 > $(APPNAME).conf.5.gz

# Install the app.
install: $(APPNAME) $(APPNAME).8.gz $(APPNAME).conf.5.gz
	install -D -m755 $(APPNAME) $(PREFIX)/sbin/$(APPNAME)
	install -D -m644 $(APPNAME).8.gz $(PREFIX)/man/man8/$(APPNAME).8.gz
	install -D -m644 $(APPNAME).conf /etc/$(APPNAME).conf
	install -D -m644 $(APPNAME).conf.5.gz $(PREFIX)/man/man5/$(APPNAME).conf.5.gz
ifeq (Linux,$(UNAME))
	install -D -m644 $(APPNAME).service /etc/systemd/system/$(APPNAME).service
#	install -D -m755 $(APPNAME).init.sh /etc/init.d/$(APPNAME)
endif
uninstall:
	rm -f $(PREFIX)/sbin/$(APPNAME)
	rm -f $(PREFIX)/man/man8/$(APPNAME).8.gz
	rm -f /etc/$(APPNAME).conf
	rm -f $(PREFIX)/man/man5/$(APPNAME).conf.5.gz
ifeq (Linux,$(UNAME))
	rm -f /etc/systemd/system/$(APPNAME).service
#	rm -f /etc/init.d/$(APPNAME)
endif

# Zap the cruft.
clean:
	rm -f *~
	rm -f *.d
	rm -f *.o
	rm -f $(APPNAME)
	rm -rf $(APPNAME).dSYM
distclean: clean
	rm -f tags
	rm -f $(APPNAME).log
	rm -f $(APPNAME).8.gz
	rm -f $(APPNAME).conf.5.gz
	rm -f $(APPNAME)-$(VERSION).zip
	rm -f $(APPNAME)-$(VERSION).tar.gz

# Git helper.
git:
	@test -d .git || git init
	git add -v $(CODE) $(DOCS) $(MISC)
	-git commit -m "$$(date -u '+AutoCommit: %Y-%m-%d@%H:%M:%S')"

# Generate an archive.
zip:
	zip -9 $(APPNAME)-$(VERSION).zip $(CODE) $(DOCS) $(MISC) &> /dev/null
tar:
	tar --xform "s#^#$(APPNAME)-$(VERSION)/#" -zcvf $(APPNAME)-$(VERSION).tar.gz $(CODE) $(DOCS) $(MISC) &> /dev/null


# vim:ts=2:sw=2:tw=120:fo=tcnq2b:foldmethod=indent
