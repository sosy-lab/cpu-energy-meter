Intel(r) Power Gadget
=====================

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


Source
------

Source (as of 2015-11-19): https://software.intel.com/en-us/articles/intel-power-gadget-20


How to use it
-------------

Prerequisites:
This tool uses the msr and cpuid kernel modules. You may have to do:
> modprobe msr

> modprobe cpuid

On RedHat, you may have to run:
> mk_msr_dev_redhat.sh

To build:
> make

To run:
> ./power_gadget [-e [sampling_delay_ms] optional]

The tool will continue counting the cumulative energy use of all supported CPUs
in the background and will report a key-value list of its measurements when it
receives SIGINT:

```
cpu_count=2
start_time=1447939429.007843
end_time=1447939436.589032
duration=7.581189
cpu0_package_Joules=100.231262
cpu0_package_mWh=27.842017
cpu0_core_Joules=20.992371
cpu0_core_mWh=5.831214
cpu0_uncore_Joules=0.000000
cpu0_uncore_mWh=0.000000
cpu0_dram_Joules=92.908188
cpu0_dram_mWh=25.807830
cpu1_package_Joules=111.545624
cpu1_package_mWh=30.984895
cpu1_core_Joules=31.105057
cpu1_core_mWh=8.640294
cpu1_uncore_Joules=0.000000
cpu1_uncore_mWh=0.000000
cpu1_dram_Joules=99.936417
cpu1_dram_mWh=27.760116
```


Known Limitations / Issues / BKMs
-----------------------------------

- The DRAM RAPL is not enabled in BIOS by default.
To enable in BIOS, go to Memory Configuration and change the mode from
'Performance' to 'Power Efficient'. Then select 'Mode 1'. This is the
VR (voltage regulator) mode of power estimation. The accuracy of this mode
is highly dependent on the OEM platform. For Intel reference platforms the
accuracy of DRAM power estimation may produce up to ~30% error.

