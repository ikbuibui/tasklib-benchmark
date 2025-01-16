#!/bin/sh

worker_counts=(8 16 32 64) # Worker counts
export n_resources=64  # Assuming n_resources stays constant
export n_repeat=15

README="$(pwd)/README.md"

update_image() {
    echo "update_image..."
    echo "sed" "s~\[$1\]([^\)]*)~[$1]($2)~g" ${README}
    sed -i "s~\[$1\]([^\)]*)~[$1]($2)~g" ${README}
}

for n_workers in "${worker_counts[@]}"; do
    export n_workers
    echo "Running for n_workers=${n_workers}..."

    pushd latency
    . ./make_plot.sh
    echo "OUTPUT=${OUTPUT}"
    update_image "Latency (Workers: ${n_workers})" "latency/${OUTPUT}"
    popd

    pushd randomGraph
    . ./scripts/bench_emplacement.sh
    update_image "Emplacement (Workers: ${n_workers})" "randomGraph/${OUTPUT}"

    export min_task_duration=200
    export max_task_duration=200

    export min_dependencies=0
    export max_dependencies=0
    . ./scripts/bench_overhead.sh
    update_image "Overhead: Independent (Workers: ${n_workers})" "randomGraph/${OUTPUT}"

    export min_dependencies=1
    export max_dependencies=1
    . ./scripts/bench_overhead.sh
    update_image "Overhead: Chains (Workers: ${n_workers})" "randomGraph/${OUTPUT}"

    export min_dependencies=1
    export max_dependencies=5
    . ./scripts/bench_overhead.sh
    update_image "Overhead: Random (Workers: ${n_workers})" "randomGraph/${OUTPUT}"

    export min_task_duration=0
    export max_task_duration=0
    . ./scripts/bench_scheduling_gap.sh
    update_image "Scheduling Gap: 0μs (Workers: ${n_workers})" "randomGraph/$OUTPUT"

    export min_task_duration=50
    export max_task_duration=50
    . ./scripts/bench_scheduling_gap.sh
    update_image "Scheduling Gap: 50μs (Workers: ${n_workers})" "randomGraph/$OUTPUT"

    export min_task_duration=25
    export max_task_duration=500
    . ./scripts/bench_scheduling_gap.sh
    update_image "Scheduling Gap: 25-500μs (Workers: ${n_workers})" "randomGraph/$OUTPUT"

    popd


    # pushd cholesky

    # matrix_size=4096 . ./make_plot.sh
    # update_image "Cholesky 128MiB (Workers: ${n_workers})" "cholesky/${OUTPUT}"

    # matrix_size=8192 . ./make_plot.sh
    # update_image "Cholesky 512MiB (Workers: ${n_workers})" "cholesky/${OUTPUT}"

    # popd

done

