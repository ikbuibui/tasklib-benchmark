#!/bin/sh

git clone --depth=1 --branch=v1.12.0 https://github.com/gabime/spdlog
mkdir -p spdlog/build
cd spdlog/build
cmake .. -DCMAKE_INSTALL_PREFIX=${BENCHMARK_DIR}/thirdparty/.install -DSPDLOG_FMT_EXTERNAL=ON
make -j install

