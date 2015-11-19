#!/bin/sh -x
# This code was taken from Pat Fei's id_cpu tool

if [ ! -d /dev/cpu/ ]
then
	mkdir /dev/cpu
fi

cpucount=`cat /proc/cpuinfo|grep processor|wc -l`

i=0
while [ $i -ne $cpucount ]
do
	if [ ! -d /dev/cpu/${i} ]
	then
		mkdir /dev/cpu/${i}
	fi

	if [ ! -c /dev/cpu/${i}/msr ]
	then
		echo "Creating /dev/cpu/"${i}"/msr"
		mknod /dev/cpu/${i}/msr c 202 ${i}
		chmod +rw /dev/cpu/${i}/msr
		echo "Creating /dev/cpu/"${i}"/cpuid"
		mknod /dev/cpu/${i}/cpuid c 203 ${i}
	fi
	i=`expr $i + 1`
done 

# RedHat EL4 64 bit all kernel versions have cpuid and msr driver built in. 
# You can double check it by looking at /boot/config* file for the kernel installed. 
# And look for CPUID and MSR driver config option. It it says 'y' then it is 
# builtin the kernel. If it says 'm', then it is a module and modprobe is needed.


