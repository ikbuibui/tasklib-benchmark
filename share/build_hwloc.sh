#!/bin/sh

git clone --depth=1 https://github.com/open-mpi/hwloc
cd hwloc
libtoolize
./autogen.sh
./configure --prefix=${BENCHMARK_DIR}/thirdparty/.install
make -j install

