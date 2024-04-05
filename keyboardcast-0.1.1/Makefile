PREFIX = /usr/local
SHAREDIR = $(PREFIX)/share/keyboardcast
APPSDIR = $(PREFIX)/share/applications
BINDIR = $(PREFIX)/bin

PKGS = gtk+-2.0 libwnck-1.0 libglade-2.0
CFLAGS = `pkg-config --cflags $(PKGS)`
LDLIBS = `pkg-config --libs $(PKGS)`
LDLIBS += -lXmu
CFLAGS += -Wall -O2 -DPREFIX=\"$(PREFIX)\"

keyboardcast: keyboardcast.o window-list.o grab-window.o

clean:
	rm -f keyboardcast *.o *.gladep *.bak

install: keyboardcast keyboardcast.glade
	install -d $(DESTDIR)$(SHAREDIR) $(DESTDIR)$(BINDIR) \
			$(DESTDIR)$(APPSDIR)
	install keyboardcast $(DESTDIR)$(BINDIR)
	install keyboardcast.glade $(DESTDIR)$(SHAREDIR)
	install keyboardcast.desktop $(DESTDIR)$(APPSDIR)
