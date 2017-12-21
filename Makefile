CC=gcc
CFLAGS=-I.

DEPS = cpuid.h intel-family.h msr.h rapl.h util.h
OBJ = cpu-energy-meter.o cpuid.o msr.o rapl.o util.o

LIBS=-lm -lcap

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

cpu-energy-meter: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

gprof: CFLAGS = -pg
gprof: all
	./cpu-energy-meter -e 100 -d 60 >/dev/null 2>&1
	gprof cpu-energy-meter > cpu-energy-meter.gprof
	rm -f gmon.out
	make clean

.PHONY: clean
clean: 
	rm -f cpu-energy-meter $(OBJ)
