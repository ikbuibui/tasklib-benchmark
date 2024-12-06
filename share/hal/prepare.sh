#!/bin/bash

export BENCHMARK_DIR=$(pwd)

load_modules() {
    module purge
    module load gcc-11.3.0-gcc-11.3.0-uhujaf2 
    module load git-2.39.1-gcc-11.3.0-makd7gp
    module load cmake-3.25.2-gcc-11.3.0-7etjlcd
    module load python-3.10.10-gcc-11.3.0-so4uylo

    module load boost-1.81.0-gcc-11.3.0-u3oct6d
    module load zlib-1.2.13-gcc-11.3.0-v3uhvno
    module load libpng-1.6.39-gcc-11.3.0-rpqwpfm
    module load pngwriter-0.7.0-gcc-11.3.0-qojchxq

    # generating plots
    module load gnuplot imagemagick-7.1.1-29-2yo2fsan3lmvyvnocx26bqxfprpay4ru inkscape

    # For Building Hwloc
    module load libtool-2.4.7-gcc-11.3.0-jekr5bf autoconf-2.69-gcc-11.3.0-rvfoaiv automake-1.16.5-gcc-11.3.0-s554yrl
}

build_dependencies() {
    cd ${BENCHMARK_DIR}/thirdparty
    if [ ! -d fmt ]; then ${BENCHMARK_DIR}/share/build_fmt.sh; fi
    if [ ! -d spdlog ]; then ${BENCHMARK_DIR}/share/build_spdlog.sh; fi
    if [ ! -d hwloc ]; then ${BENCHMARK_DIR}/share/build_hwloc.sh; fi
    if [ ! -d quark ]; then ${BENCHMARK_DIR}/share/build_quark.sh; fi
    if [ ! -d redGrapes ]; then git submodule update --init --recursive; fi
    cd -
}

export_paths() {
    export rg_DIR=${BENCHMARK_DIR}/thirdparty/rg
    export redGrapes_DIR=${BENCHMARK_DIR}/thirdparty/redGrapes
    export fmt_DIR=${BENCHMARK_DIR}/thirdparty/.install/lib64/cmake/fmt/
    export spdlog_DIR=${BENCHMARK_DIR}/thirdparty/.install/lib64/cmake/spdlog/    
    export HWLOC_ROOT=${BENCHMARK_DIR}/thirdparty/.install/
    export LD_LIBRARY_PATH=${BENCHMARK_DIR}/thirdparty/.install/lib:${LD_LIBRARY_PATH}
    export CMAKE_PREFIX_PATH=${BENCHMARK_DIR}/thirdparty/.install:${CMAKE_PREFIX_PATH}
}

benchmark_job() {
    queue=$1
    cmd=$2
    srun --time=10:00:00   --nodes=1  --tasks-per-node=1  --cpus-per-task=128  --mem=0  -p $queue --pty $cmd
}

load_modules
export_paths

export -f build_dependencies

