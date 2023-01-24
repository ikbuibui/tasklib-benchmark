#!/bin/sh

n_workers=${n_workers-64}
min_task_duration=${min_task_duration-0}
max_task_duration=${max_task_duration-0}
n_resources=$n_workers
n_repeat=${n_repeat-15}

build()
{
    mkdir -p build
    cd build
    cmake ..
    make -j
    cd ..
}

run()
{
    mkdir -p data/schedulingGap
    for lib in quark redgrapes superglue;
    do
	truncate -s 0 data/schedulingGap/${lib}
	for n_tasks in 1024 2048 4096 8192;
	do
	    DATA=""
	    ARGS="$n_tasks $n_resources 1 1 $min_task_duration $max_task_duration $n_workers true"

	    for i in $(seq $n_repeat);
	    do
		echo $lib $ARGS $i
		OUTPUT=$(xargs ./build/$lib <<< "$ARGS $i")

		grep -s 'success' <<< $OUTPUT >/dev/null || exit
		GAP=$(grep -Po 'scheduling gap \K[.0-9]*' <<< $OUTPUT)

		echo "gap = " $GAP

		DATA="$GAP $DATA"
	    done
	    
	    echo $DATA
	    MIN=$(awk '{min = $1; for(i=1;i<=NF;i++) { if($i < min) { min = $i } } } END { printf "%f\n", min }' <<< $DATA)
	    MAX=$(awk '{max = $1; for(i=1;i<=NF;i++) { if($i > max) { max = $i } } } END { printf "%f\n", max }' <<< $DATA)
	    AVG=$(awk '{sum = 0; for(i=1;i<=NF;i++) { sum += $i } } END { printf "%f\n", sum/NF }' <<< $DATA)
	    VAR=$(awk "{ varsum = 0; for(i=1;i<=NF;i++) { varsum += (\$i-$AVG)*(\$i-$AVG); } } END { printf \"%f\n\", varsum/NF }" <<< $DATA)
	    SIG=$(bc -l <<< "sqrt($VAR)")

	    echo "min=$MIN, max=$MAX, avg=$AVG, sigma=$SIG"
	    echo "$n_tasks $AVG $MIN $MAX $SIG" >> data/schedulingGap/${lib}
	done
    done
}

plot()
{
    mkdir -p plots

    OUTPUT="plots/scheduling_gap_dur${min_task_duration}_${max_task_duration}_w${n_workers}.png"
    TITLE="${n_resources} chains,\\\n ${n_workers} workers,\\\n ${min_task_duration} - ${max_task_duration} μs task duration,\\\n Host: $(cat /etc/hostname)"
    LABEL_X="#tasks"
    LABEL_Y="avg scheduling gap (μs)"
    LOGX=1

    . ../plot.sh <<< "data/schedulingGap/quark Quark #86C4FF #006DD5
data/schedulingGap/superglue SuperGlue #88F176 #20D500
data/schedulingGap/redgrapes RedGrapes #C976F1 #670496"
}

build
run
plot

