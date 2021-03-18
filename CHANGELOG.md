<!--
This file is part of CPU Energy Meter,
a tool for measuring energy consumption of Intel CPUs:
https://github.com/sosy-lab/cpu-energy-meter

SPDX-FileCopyrightText: 2018-2021 Dirk Beyer <https://www.sosy-lab.org>

SPDX-License-Identifier: BSD-3-Clause
-->

# Changelog

## CPU Energy Meter 1.1

- Now works on systems with more than one CPU socket.
- Now works on systems where the Linux kernel assigns non-consecutive core ids.
- Privileges were not dropped as expected after initialization when running as non-root user.
- Bugs fixed that could lead to broken measurements if compiled with high optimization levels
- Undocumented behavior removed: SIGQUIT does no longer produce result output
  (use SIGINT as documented instead)
- Better error handling
- More informative debug output with `-d`

## CPU Energy Meter 1.0

First official release.
