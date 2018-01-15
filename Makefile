VERSION := 0.1

CC = gcc
CFLAGS = -I. -DVERSION=\"$(VERSION)\"
LIBS=-lm -lcap

TARGET = cpu-energy-meter
SOURCES = cpu-energy-meter.c cpuid.c msr.c rapl.c util.c
HEADERS = cpuid.h intel-family.h msr.h rapl.h util.h
OBJECTS = $(SOURCES:.c=.o)
AUX = README.md LICENSE

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
	-rm -f $(OBJECTS)

.PHONY: install
install: all
	install -d $(BINDIR)
	install -g msr -m 2611 $(TARGET) $(BINDIR)
	setcap cap_sys_rawio=ep $(BINDIR)/$(TARGET)
	test -d $(MODULEDIR) || mkdir -p $(MODULEDIR)
	rm -f $(MODULEDIR)/$(TARGET).conf
	@echo "# This file was created by $(TARGET)." >> $(MODULEDIR)/$(TARGET).conf
	@echo "# The kernel module is required in order for $(TARGET) to be executed \
	successfully." >> $(MODULEDIR)/$(TARGET).conf
	echo "msr" >> $(MODULEDIR)/$(TARGET).conf
	systemctl restart systemd-modules-load.service

.PHONY: uninstall
uninstall:
	-rm -f $(BINDIR)/$(TARGET)
	-rm -f $(MODULEDIR)/$(TARGET).conf
	-systemctl restart systemd-modules-load.service

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
	-rm -rf $(TARGET)-$(VERSION)
	mkdir $(TARGET)-$(VERSION)
	cp $(SOURCES) $(HEADERS) Makefile $(AUX) $(TARGET)-$(VERSION)
	tar cf - $(TARGET)-$(VERSION) | gzip -9c > $(TARGET)-$(VERSION).tar.gz
	-rm -rf $(TARGET)-$(VERSION)

.PHONY: distclean
distclean: clean
	-rm -f $(TARGET)
	-rm -rf $(TARGET)-[0-9]*.[0-9]*
	-rm -f $(TARGET)-[0-9]*.[0-9]*.tar.gz

#.PHONY: test
#test: $(TARGET)
#	echo # TODO: NYI. For an example, see e.g.
#	echo # http://makepp.sourceforge.net/1.19/makepp_tutorial.html
