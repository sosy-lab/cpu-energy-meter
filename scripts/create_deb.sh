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
dpkg-buildpackage --build=source
echo "No upload manually to GitHub and to PPA with "dput ppa:sosy-lab/benchmarking ..._source.changes"
