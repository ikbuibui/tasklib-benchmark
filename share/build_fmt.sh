#!/bin/sh

git clone --depth=1 --branch=10.1.1 https://github.com/fmtlib/fmt
mkdir -p fmt/build
cd fmt/build
cmake .. -DCMAKE_INSTALL_PREFIX=${BENCHMARK_DIR}/thirdparty/.install
make -j install

