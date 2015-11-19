Intel(r) Power Gadget    {#mainpage}
=======================

Intel(r) Power Gadget is a software utility and library, which allows developers 
to monitor power at very fine time granularities (few tens of milliseconds). 
Power monitoring is available for the following power domains: 
- per package domain (CPU socket)
- per core domain (all the cpu cores on a package)
- per uncore domain (uncore components, e.g. integrated graphics, client parts
  only) 
- per memory node (memory local to a package, server parts only) 
To do this, the tool uses an architected capability in
Intel(r) processors which is called RAPL (Runtime Average Power Limiting).
RAPL is available on Intel(r) codename Sandy Bridge and later processors. 


How to use it
-----------------------------------

Prerequisites: 
This tool uses the msr and cpuid kernel modules. You may have to do: 
> modprobe msr 

> modprobe cpuid 

On RedHat, you may have to run: 
> mk_msr_dev_redhat.sh

To build: 
> make

To run: 
> ./power_gadget [-e [sampling_delay_ms ] optional] -d duration]


Known Limitations / Issues / BKMs
-----------------------------------

- The DRAM RAPL is not enabled in BIOS by default.
To enable in BIOS, go to Memory Configuration and change the mode from
'Performance' to 'Power Efficient'. Then select 'Mode 1'. This is the 
VR (voltage regulator) mode of power estimation. The accuracy of this mode
is highly dependent on the OEM platform. For Intel reference platforms the 
accuracy of DRAM power estimation may produce up to ~30% error. 

 
