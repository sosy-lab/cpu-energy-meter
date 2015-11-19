CFLAGS=-g

all: rapl_lib_shared rapl_lib_static power_gadget_static

rapl_lib_shared: 
	gcc $(CFLAGS) -fpic -c msr.c cpuid.c rapl.c 
	gcc $(CFLAGS) -shared -o librapl.so msr.o cpuid.o rapl.o

rapl_lib_static: 
	gcc $(CFLAGS) -c msr.c cpuid.c rapl.c 
	ar rcs librapl.a msr.o cpuid.o rapl.o

power_gadget_static: 
	gcc $(CFLAGS) power_gadget.c -I. -L. -o power_gadget ./librapl.a -lm

power_gadget: 
	gcc $(CFLAGS) power_gadget.c -I. -L. -lrapl -o power_gadget -lm

gprof: CFLAGS = -pg
gprof: all
	./power_gadget -e 100 -d 60 >/dev/null 2>&1
	gprof power_gadget > power_gadget.gprof
	rm -f gmon.out
	make clean

clean: 
	rm -f power_gadget librapl.so librapl.a msr.o cpuid.o rapl.o 
