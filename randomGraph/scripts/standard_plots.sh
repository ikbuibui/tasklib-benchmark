#!/bin/sh

export n_workers=64
export n_resources=$n_workers
export n_repeat=15

. ./scripts/bench_emplacement.sh

##
## Case A: No Dependencies
##
export min_dependencies=0
export max_dependencies=0


export min_task_duration=0
export max_task_duration=0
. ./scripts/bench_overhead.sh

export min_task_duration=50
export max_task_duration=50
. ./scripts/bench_overhead.sh
OUT_A=$OUTPUT



##
## Case B: Chains
##
export min_dependencies=1
export max_dependencies=1


export min_task_duration=0
export max_task_duration=0
. ./scripts/bench_overhead.sh

export min_task_duration=25
export max_task_duration=500
. ./scripts/bench_overhead.sh

export min_task_duration=50
export max_task_duration=50
. ./scripts/bench_overhead.sh
OUT_B=$OUTPUT



##
## Case C: Random Graph, fixed duration
##
export min_task_duration=50
export max_task_duration=50


export min_dependencies=1
export max_dependencies=5
. ./scripts/bench_overhead.sh
OUT_C=$OUTPUT

##
## Case D: Random Graph, random duration
##
export min_task_duration=25
export max_task_duration=500
export min_dependencies=1
export max_dependencies=5
. ./scripts/bench_overhead.sh
OUT_D=$OUTPUT



##
## Montage
##
montage $OUT_A $OUT_B $OUT_C $OUT_D -geometry +2+2 bench_montage.png



export min_task_duration=0
export max_task_duration=0
. ./scripts/bench_scheduling_gap.sh

export min_task_duration=50
export max_task_duration=50
. ./scripts/bench_scheduling_gap.sh

export min_task_duration=25
export max_task_duration=500
. ./scripts/bench_scheduling_gap.sh


