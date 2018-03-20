VERSION = 1.1-dev

DESTDIR :=
PREFIX := /usr/local
BINDIR = $(PREFIX)/bin

#TODO: move unity into a subdir of cpu-energy-meter? Ask PW
PATH_UNITY = ../unity/src/
PATH_SOURCEFILES = src/
PATH_TESTFILES = test/
PATH_BUILD = build/
PATH_OBJECTS = build/objs/
PATH_RESULTS = build/results/

BUILD_PATHS = $(PATH_BUILD) $(PATH_OBJECTS) $(PATH_RESULTS)

SRC_TESTFILES = $(wildcard $(PATH_TESTFILES)*.c)

CC =gcc -c
LINK =gcc
CFLAGS =-I. -I$(PATH_UNITY) -I$(PATH_SOURCEFILES)
LIBS =-lm -lcap

TARGET_BIN = cpu-energy-meter
_SOURCES = cpu-energy-meter.c cpuid.c msr.c rapl.c util.c
SOURCES = $(patsubst %,$(PATH_SOURCEFILES)%,$(_SOURCES)) #convert to $PATH_SOURCEFILES/_SOURCES
_HEADERS = cpuid.h intel-family.h msr.h rapl.h util.h
HEADERS = $(patsubst %,$(PATH_SOURCEFILES)%,$(_HEADERS)) #convert to $PATH_SOURCEFILES/_HEADERS
_OBJECTS = $(_SOURCES:.c=.o)
OBJECTS = $(patsubst %,$(PATH_OBJECTS)%,$(_OBJECTS)) #convert to $PATH_OBJECTS/_OBJECTS
AUX = README.md LICENSE

.PHONY: all
all:	$(BUILD_PATHS) $(TARGET_BIN)

# Create object files from PATH_SOURCEFILES/*.c in PATH_OBJECTS/*.o
$(PATH_OBJECTS)%.o:: $(PATH_SOURCEFILES)%.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

# Create the cpu-energy-meter binary.
$(TARGET_BIN): $(OBJECTS)
	$(LINK) $(CFLAGS) -o $@ $^ $(LIBS)


# Tell the compiler to look for all Test*.c files in SRC_TESTFILES dir, and invent a
# file named Test*.txt in the PATH_RESULTS folder
# Ex.: test/*.c -> build/results/Test*.txt
RESULTS = $(patsubst $(PATH_TESTFILES)Test%.c,$(PATH_RESULTS)Test%.txt,$(SRC_TESTFILES) )

PASSED = `grep -s PASS $(PATH_RESULTS)*.txt`
FAIL = `grep -s FAIL $(PATH_RESULTS)*.txt`
IGNORE = `grep -s IGNORE $(PATH_RESULTS)*.txt`

.PHONY: test
test: $(BUILD_PATHS) $(RESULTS)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"

# Run executable ('$<') and pipe the combined output of stdout and stderr \
# to the specified dir given in $@ (i.e., PATH_BUILD/*.out)
$(PATH_RESULTS)%.txt: $(PATH_BUILD)%.out
	-./$< > $@ 2>&1

$(PATH_BUILD)Test%.out: $(PATH_OBJECTS)Test%.o $(PATH_OBJECTS)%.o $(PATH_UNITY)unity.o
	$(LINK) -o $@ $^ $(LIBS)

$(PATH_OBJECTS)%.o:: $(PATH_TESTFILES)%.c
	$(CC) $(CFLAGS) $< -o $@

#$(PATH_OBJECTS)%.o:: $(PATH_SOURCEFILES)%.c $(PATH_SOURCEFILES)%.h
#	$(CC) $(CFLAGS) $< -o $@

$(PATH_OBJECTS)%.o:: $(PATH_UNITY)%.c $(PATH_UNITY)%.h
	$(CC) $(CFLAGS) $< -o $@

$(PATH_BUILD):
	mkdir -p $(PATH_BUILD)

$(PATH_OBJECTS):
	mkdir -p $(PATH_OBJECTS)

$(PATH_RESULTS):
	mkdir -p $(PATH_RESULTS)

.PHONY: clean
clean:
	rm -f $(TARGET_BIN)
	rm -rf $(PATH_BUILD)

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
	cp $(SOURCES) $(HEADERS) Makefile $(AUX) $(DESTDIR)$(TARGET_BIN)-$(VERSION)
	tar cf - $(DESTDIR)$(TARGET_BIN)-$(VERSION) | gzip -9c > $(DESTDIR)$(TARGET_BIN)-$(VERSION).tar.gz
	-rm -rf $(DESTDIR)$(TARGET_BIN)-$(VERSION)

.PHONY: distclean
distclean: clean
	rm -f $(DESTDIR)$(TARGET_BIN)
	-rm -rf $(DESTDIR)$(TARGET_BIN)-[0-9]*.[0-9]*
	-rm -f $(DESTDIR)$(TARGET_BIN)-[0-9]*.[0-9]*.tar.gz

# Keep the following intermediate files after make has been executed
.PRECIOUS: $(PATH_BUILD)Test%.out
.PRECIOUS: $(PATH_OBJECTS)%.o
.PRECIOUS: $(PATH_RESULTS)%.txt
