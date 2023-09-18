#!/bin/sh

export n_workers=64
export n_resources=$n_workers
export n_repeat=1

README="$(pwd)/README.md"

update_image() {
    echo "update_image..."
    echo "sed" "s~\[$1\](.*)~[$1]($2)~g" ${README}
    sed -i "s~\[$1\](.*)~[$1]($2)~g" ${README}
}

pushd latency
. ./make_plot.sh
echo "OUTPUT=${OUTPUT}"
update_image "Latency" "latency/${OUTPUT}"
popd

pushd randomGraph
./scripts/bench_emplacement.sh
update_image "Emplacement" "randomGraph/${OUTPUT}"

./scripts/bench_overhead.sh
update_image "Overhead: Independent" "randomGraph/${OUTPUT}"

./scripts/bench_overhead.sh
update_image "Overhead: Chains" "randomGraph/${OUTPUT}"

./scripts/bench_overhead.sh
update_image "Overhead: Random" "randomGraph/${OUTPUT}"

export min_task_duration=0
export max_task_duration=0
./scripts/bench_scheduling_gap.sh
update_image "Scheduling Gap: 0μs" "randomGraph/$OUTPUT"

export min_task_duration=50
export max_task_duration=50
./scripts/bench_scheduling_gap.sh
update_image "Scheduling Gap: 50μs" "randomGraph/$OUTPUT"

export min_task_duration=25
export max_task_duration=500
./scripts/bench_scheduling_gap.sh
update_image "Scheduling Gap: 25-500μs" "randomGraph/$OUTPUT"

popd


pushd cholesky

matrix_size=4096 . ./make_plot.sh
update_image "Cholesky 128MiB" "cholesky/${OUTPUT}"

matrix_size=8192 . ./make_plot.sh
update_image "Cholesky 512MiB" "cholesky/${OUTPUT}"

popd

