VERSION = 0.9

CC = gcc
CFLAGS = -I.
LIBS =-lm -lcap

TARGET = cpu-energy-meter
SOURCES = cpu-energy-meter.c cpuid.c msr.c rapl.c util.c
HEADERS = cpuid.h intel-family.h msr.h rapl.h util.h
OBJECTS = $(SOURCES:.c=.o)
AUX = README.md LICENSE

DESTDIR :=
PREFIX := /usr/local
BINDIR = $(PREFIX)/bin
MODULEDIR = /etc/modules-load.d

.PHONY: all
all:	$(TARGET)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	-rm -f $(TARGET) $(OBJECTS)

.PHONY: install
install: all
	install -d $(DESTDIR)$(BINDIR)
	install $(TARGET) $(DESTDIR)$(BINDIR)

.PHONY: uninstall
uninstall:
	-rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

.PHONY: gprof 	# outdated functionality that is currently broken;
		# will be fixed in a future update
gprof: CFLAGS = -pg
gprof: all
	./cpu-energy-meter -e 100 -d 60 >/dev/null 2>&1
	gprof cpu-energy-meter > cpu-energy-meter.gprof
	rm -f gmon.out
	make clean

.PHONY: dist
dist:
	-rm -rf $(DESTDIR)$(TARGET)-$(VERSION)
	mkdir $(DESTDIR)$(TARGET)-$(VERSION)
	cp $(SOURCES) $(HEADERS) Makefile $(AUX) $(DESTDIR)$(TARGET)-$(VERSION)
	tar cf - $(DESTDIR)$(TARGET)-$(VERSION) | gzip -9c > $(DESTDIR)$(TARGET)-$(VERSION).tar.gz
	-rm -rf $(DESTDIR)$(TARGET)-$(VERSION)

.PHONY: distclean
distclean: clean
	-rm -f $(DESTDIR)$(TARGET)
	-rm -rf $(DESTDIR)$(TARGET)-[0-9]*.[0-9]*
	-rm -f $(DESTDIR)$(TARGET)-[0-9]*.[0-9]*.tar.gz

#.PHONY: test
#test: $(TARGET)
#	echo # TODO: NYI. For an example, see e.g.
#	echo # http://makepp.sourceforge.net/1.19/makepp_tutorial.html
