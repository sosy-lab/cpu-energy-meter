VERSION = 1.1-dev

DESTDIR :=
PREFIX := /usr/local
BINDIR = $(PREFIX)/bin

SRC_DIR = src/
TEST_DIR = test/
BUILD_DIR = build/
OBJ_DIR = build/obj/
BUILD_PATHS = $(BUILD_DIR) $(OBJ_DIR)

# The parameters below are required by CMock
TEST_BUILD_DIR ?= ${BUILD_DIR}/test
TEST_MAKEFILE = ${TEST_BUILD_DIR}/MakefileTestSupport
OBJ = ${OBJ_DIR}

CC =gcc
CFLAGS =-I. -I$(SRC_DIR)
LDFLAGS =-Wl,--no-as-needed -lm -lcap
LIBS =-lm -lcap

TARGET_BIN = cpu-energy-meter
_SOURCES = cpu-energy-meter.c cpuid.c msr.c rapl.c util.c
SOURCES = $(patsubst %,$(SRC_DIR)%,$(_SOURCES)) #convert to $SRC_DIR/_SOURCES
_HEADERS = cpuid.h intel-family.h msr.h rapl.h util.h
HEADERS = $(patsubst %,$(SRC_DIR)%,$(_HEADERS)) #convert to $SRC_DIR/_HEADERS
TESTFILES = $(wildcard $(TEST_DIR)*.c)
_OBJECTS = $(_SOURCES:.c=.o)
OBJECTS = $(patsubst %,$(OBJ_DIR)%,$(_OBJECTS)) #convert to $OBJ_DIR/_OBJECTS
AUX = README.md LICENSE

.PHONY: default
default: all

.PHONY: all
all: $(BUILD_PATHS) $(TARGET_BIN)

# Create object files from SRC_DIR/*.c in OBJ_DIR/*.o
$(OBJ_DIR)%.o:: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create the cpu-energy-meter binary.
$(TARGET_BIN): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

.PHONY: setup
# Needs to be executed with root-rights ('sudo make setup')
setup: $(TARGET_BIN)
	chgrp msr $<
	chmod 2711 $<
	setcap cap_sys_rawio=ep $<

.PHONY: test
test: $(BUILD_PATHS)
	ruby scripts/create_makefile.rb

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

.PHONY: clean
clean:
	rm -f $(TARGET_BIN)
	rm -rf $(BUILD_DIR)

.PHONY: install
install: all
	install -d $(DESTDIR)$(BINDIR)
	install $(TARGET_BIN) $(DESTDIR)$(BINDIR)

.PHONY: uninstall
uninstall:
	-rm -f $(DESTDIR)$(BINDIR)/$(TARGET_BIN)

.PHONY: gprof   # outdated functionality that is currently broken;
                # will be fixed in a future update
gprof: CFLAGS = -pg
gprof: all
	./cpu-energy-meter -e 100 -d 60 >/dev/null 2>&1
	gprof cpu-energy-meter > cpu-energy-meter.gprof
	rm -f gmon.out
	make clean

.PHONY: dist
dist:
	-rm -rf $(DESTDIR)$(TARGET_BIN)-$(VERSION)
	mkdir $(DESTDIR)$(TARGET_BIN)-$(VERSION)
	cp --parents $(SOURCES) $(HEADERS) $(TESTFILES) Makefile $(AUX) $(DESTDIR)$(TARGET_BIN)-$(VERSION)
	tar cf - $(DESTDIR)$(TARGET_BIN)-$(VERSION) | gzip -9c > $(DESTDIR)$(TARGET_BIN)-$(VERSION).tar.gz
	-rm -rf $(DESTDIR)$(TARGET_BIN)-$(VERSION)

.PHONY: distclean
distclean: clean
	rm -f $(DESTDIR)$(TARGET_BIN)
	-rm -rf $(DESTDIR)$(TARGET_BIN)-[0-9]*.[0-9]*
	-rm -f $(DESTDIR)$(TARGET_BIN)-[0-9]*.[0-9]*.tar.gz

# Keep the following intermediate files after make has been executed
.PRECIOUS: $(OBJ_DIR)%.o

-include ${TEST_MAKEFILE}

