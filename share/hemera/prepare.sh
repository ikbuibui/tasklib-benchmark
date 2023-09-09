#!/bin/sh

export BENCHMARK_DIR=$(pwd)

load_modules() {
    module load git cmake/3.20 gcc/11.2.0
    module load python/3.10.4
    module load boost

    module load openmpi openblas/rome/0.3.17
    module load zlib libpng/1.6.35
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
    export LD_LIBRARY_PATH=${BENCHMARK_DIR}/thirdparty/.install/lib:${LD_LIBRARY_PATH}
}

benchmark_job() {
    queue=$1
    cmd=$2
    srun --time=10:00:00   --nodes=1  --tasks-per-node=1  --cpus-per-task=128  --mem=0  -p $queue --pty $cmd
}

load_modules
export_paths

