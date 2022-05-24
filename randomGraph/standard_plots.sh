
export task_duration=25

## Case A: No Dependencies
export n_resources=0
export min_dependencies=0
export max_dependencies=0
export n_workers=4
./make_plot.sh

## Case B: Chains
export n_resources=4
export min_dependencies=1
export max_dependencies=1
export n_workers=4
./make_plot.sh

## Case C: Random Graph
export n_resources=20
export min_dependencies=1
export max_dependencies=5
export n_workers=4
./make_plot.sh

## Case D: Many Workers
export n_resources=20
export min_dependencies=1
export max_dependencies=5
export n_workers=32
./make_plot.sh

