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
dh_make -y -s --createorig -p cpu-energy-meter -c mit -f "../$TAR" || true
dpkg-buildpackage -us -uc
