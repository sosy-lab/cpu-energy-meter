CFLAGS=-g

all: rapl_lib_shared rapl_lib_static cpu-energy-meter_static

rapl_lib_shared: 
	gcc $(CFLAGS) -fpic -c msr.c cpuid.c rapl.c util.c
	gcc $(CFLAGS) -shared -o librapl.so msr.o cpuid.o rapl.o util.o

rapl_lib_static: 
	gcc $(CFLAGS) -c msr.c cpuid.c rapl.c util.c
	ar rcs librapl.a msr.o cpuid.o rapl.o util.o

cpu-energy-meter_static: 
	gcc $(CFLAGS) cpu-energy-meter.c -I. -L. -o cpu-energy-meter ./librapl.a -lm -lcap

cpu-energy-meter:
	gcc $(CFLAGS) cpu-energy-meter.c -I. -L. -lrapl -o cpu-energy-meter -lm -lcap

gprof: CFLAGS = -pg
gprof: all
	./cpu-energy-meter -e 100 -d 60 >/dev/null 2>&1
	gprof cpu-energy-meter > cpu-energy-meter.gprof
	rm -f gmon.out
	make clean

clean: 
	rm -f cpu-energy-meter librapl.so librapl.a msr.o cpuid.o rapl.o 
