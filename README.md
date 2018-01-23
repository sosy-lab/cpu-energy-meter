CPU Energy Meter
=====================

CPU Energy Meter is a software utility and library, which allows to monitor
power at very fine time granularities (few tens of milliseconds).
Power monitoring is available for the following power domains:
- per package domain (CPU socket)
- per core domain (all the cpu cores on a package)
- per uncore domain (uncore components, e.g. integrated graphics, client parts
  only)
- per memory node (memory local to a package, server parts only)
- per platform (all devices in the plattform that receive power from integraded
  power delivery mechanism, e.g., processor cores, SOC, memory, add-on or
  peripheral devices)
To do this, the tool uses an architected capability in
Intel(r) processors which is called RAPL (Runtime Average Power Limiting).
RAPL is available on Intel(r) codename Sandy Bridge and later processors.


How to use it
-------------

Prerequisites:
This tool uses the msr and cpuid kernel modules. You may have to do:
> modprobe msr

> modprobe cpuid

To build:
> make

To run:
> ./cpu-energy-meter [-e [sampling_delay_ms] optional] [-r optional]

The tool will continue counting the cumulative energy use of all supported CPUs
in the background and will report a key-value list of its measurements when it
receives SIGINT:

```
+--------------------------------------+
| CPU-Energy-Meter            Socket 0 |
+--------------------------------------+
Duration                  2.504502 sec
Package                   3.769287 Joule
Core                      0.317749 Joule
Uncore                    0.010132 Joule
DRAM                      0.727783 Joule
PSYS                     29.792603 Joule
```

Optionally, the tool can be executed together with a '-r'-tag to print the
output as a raw list:

```
cpu_count=1
duration_seconds=3.241504
cpu0_package_joules=4.971924
cpu0_core_joules=0.461182
cpu0_uncore_joules=0.053406
cpu0_dram_joules=0.953979
cpu0_psys_joules=38.904785
```


