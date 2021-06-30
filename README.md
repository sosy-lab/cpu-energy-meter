<!--
This file is part of CPU Energy Meter,
a tool for measuring energy consumption of Intel CPUs:
https://github.com/sosy-lab/cpu-energy-meter

SPDX-FileCopyrightText: 2012 Intel Corporation
SPDX-FileCopyrightText: 2015-2021 Dirk Beyer <https://www.sosy-lab.org>

SPDX-License-Identifier: BSD-3-Clause
-->

CPU Energy Meter
================

[![Build Status](https://gitlab.com/sosy-lab/software/cpu-energy-meter/badges/main/pipeline.svg)](https://gitlab.com/sosy-lab/software/cpu-energy-meter/pipelines)
[![BSD-3-Clause License](https://img.shields.io/badge/license-BSD--3--clause-brightgreen.svg)](https://github.com/sosy-lab/cpu-energy-meter/blob/main/LICENSE)
[![Releases](https://img.shields.io/github/release/sosy-lab/cpu-energy-meter.svg)](https://github.com/sosy-lab/cpu-energy-meter/releases)
[![DOI via Zenodo](https://zenodo.org/badge/46493895.svg)](https://zenodo.org/badge/latestdoi/46493895)


CPU Energy Meter is a Linux tool that allows to monitor power consumption of Intel CPUs
at fine time granularities (few tens of milliseconds).
Power monitoring is available for the following power domains:
- per package domain (CPU socket)
- per core domain (all the CPU cores on a package)
- per uncore domain (uncore components, e.g., integrated graphics on client CPUs)
- per memory node (memory local to a package, server CPUs only)
- per platform (all devices in the platform that receive power from integrated
  power delivery mechanism, e.g., processor cores, SOC, memory, add-on or
  peripheral devices)

To do this, the tool uses a feature of Intel CPUs that is called [RAPL (Running Average Power Limit)](https://en.wikipedia.org/wiki/Running_average_power_limit),
which is documented in the [Intel Software Developers Manual](https://software.intel.com/en-us/articles/intel-sdm), Volume 3B Chapter 14.9.
RAPL is available on CPUs from the generation [Sandy Bridge](https://en.wikipedia.org/wiki/Sandy_Bridge) and later.
Because CPU Energy Meter uses the maximal possible measurement interval
(depending on the hardware this is between a few minutes and an hour),
it causes negligible overhead.

CPU Energy Meter is a fork of the [Intel Power Gadget](https://software.intel.com/en-us/articles/intel-power-gadget-20)
and developed at the [Software Systems Lab](https://www.sosy-lab.org)
of the [Ludwig-Maximilians-Universität München (LMU Munich)](https://www.uni-muenchen.de)
under the [BSD-3-Clause License](https://github.com/sosy-lab/cpu-energy-meter/blob/main/LICENSE).


Installation
------------

For Debian or Ubuntu the easiest way is to install from our [PPA](https://launchpad.net/~sosy-lab/+archive/ubuntu/benchmarking):

    sudo add-apt-repository ppa:sosy-lab/benchmarking
    sudo apt install cpu-energy-meter

Alternatively, you can download our `.deb` package from [GitHub](https://github.com/sosy-lab/cpu-energy-meter/releases)
and install it with `apt install ./cpu-energy-meter*.deb`.

Dependencies of CPU Energy Meter are [libcap](https://sites.google.com/site/fullycapable/),
which is available on most Linux distributions in package `libcap` (e.g., Fedora)
or `libcap2` (e.g, Debian and Ubuntu: `sudo apt install libcap2`),
and a Linux kernel with the MSR and CPUID modules (available by default)

Alternatively, for running CPU Energy Meter from source (quick and dirty):

    sudo apt install libcap-dev
    sudo modprobe msr
    sudo modprobe cpuid
    make
    sudo ./cpu-energy-meter

It is also possible (and recommended) to run CPU Energy Meter without root.
To do so, the following needs to be done:

- Load kernel modules `msr` and `cpuid`.
- Add a group `msr`.
- Add a Udev rule that grants access to `/dev/cpu/*/msr` to group `msr` ([example](https://github.com/sosy-lab/cpu-energy-meter/blob/main/debian/additional_files/59-msr.rules)).
- Run `chgrp msr`, `chmod 2711`, and `setcap cap_sys_rawio=ep` on the binary (`make setup` is a shortcut for this).

The provided Debian package in our [PPA](https://launchpad.net/~sosy-lab/+archive/ubuntu/benchmarking)
and on [GitHub](https://github.com/sosy-lab/cpu-energy-meter/releases) does these steps automatically
and lets all users execute CPU Energy Meter.

How to use it
-------------

    cpu-energy-meter [-d] [-e sampling_delay_ms] [-r]

The tool will continue counting the cumulative energy use of all supported CPUs
in the background and will report a key-value list of its measurements when it
receives SIGINT (Ctrl+C):

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

To get intermediate measurements, send signal `USR1` to the process.

Optionally, the tool can be executed with parameter `-r`
to print the output as a raw (easily parsable) list:

```
cpu_count=1
duration_seconds=3.241504
cpu0_package_joules=4.971924
cpu0_core_joules=0.461182
cpu0_uncore_joules=0.053406
cpu0_dram_joules=0.953979
cpu0_psys_joules=38.904785
```

The parameter `-d` adds debug output.
By default, CPU Energy Meter computes the necessary measurement interval automatically,
this can be overridden with the parameter `-e`.
