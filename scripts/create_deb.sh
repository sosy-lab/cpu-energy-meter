# This file is part of CPU Energy Meter,
# a tool for measuring energy consumption of Intel CPUs:
# https://github.com/sosy-lab/cpu-energy-meter
#
# SPDX-FileCopyrightText: 2018-2021 Dirk Beyer <https://www.sosy-lab.org>
#
# SPDX-License-Identifier: BSD-3-Clause

#/bin/bash

set -euo pipefail

# Detect version
eval $(grep ^VERSION Makefile | sed -e 's/\s//g')

TAR="cpu-energy-meter-$VERSION.tar.gz"

: "${USER:=root}" # provide default for $USER
export USER

make dist
tar xf "$TAR"
cd "cpu-energy-meter-$VERSION/"
cp -a ../debian .
dch -v "$VERSION-1" "New upstream version"
dh_make -y -s --createorig -p cpu-energy-meter -c bsd -f "../$TAR" || true
dpkg-buildpackage --build=binary --no-sign
dpkg-buildpackage --build=source --no-sign
echo 'Now upload manually to GitHub and to PPA with "dput ppa:sosy-lab/benchmarking ..._source.changes"'
