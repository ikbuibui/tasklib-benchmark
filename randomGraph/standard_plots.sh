
export task_duration=25
export n_repeat=5

## Case A: No Dependencies
export n_resources=0
export min_dependencies=0
export max_dependencies=0
export n_workers=4
. ./make_plot.sh
OUT_A=$OUTPUT

## Case B: Chains
export n_resources=4
export min_dependencies=1
export max_dependencies=1
export n_workers=4
. ./make_plot.sh
OUT_B=$OUTPUT

## Case C: Random Graph
export n_resources=8
export min_dependencies=1
export max_dependencies=5
export n_workers=8
. ./make_plot.sh
OUT_C=$OUTPUT

## Case D: Many Workers
export n_resources=16
export min_dependencies=1
export max_dependencies=5
export n_workers=16
. ./make_plot.sh
OUT_D=$OUTPUT

montage $OUT_A $OUT_B $OUT_C $OUT_D -geometry +2+2 bench_montage.png

