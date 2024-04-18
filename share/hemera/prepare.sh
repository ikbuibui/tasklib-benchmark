#!/bin/sh

export BENCHMARK_DIR=$(pwd)

load_modules() {
    module purge
    module load git
    module load cmake/3.26.1
    module load python/3.10.4

    module load gcc/11.2.0
    module load boost/1.79.0
    module load openblas/rome/0.3.17
    module load openmpi/4.1.1-rome
    module load zlib/1.2.11
    module load libpng/1.6.35
    module load pngwriter/0.7.0

    # generating plots
    module load gnuplot imagemagick inkscape

    # For Building Hwloc
    module load libtool autoconf automake
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

