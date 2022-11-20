#!/bin/sh

n_resources=${n_resources-0}
min_dependencies=${min_dependencies-0}
max_dependencies=${max_dependencies-0}
min_task_duration=${min_task_duration-5}
max_task_duration=${max_task_duration-5}
n_workers=${n_workers-8}
block_exec=${block_exec-"false"}
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
    for lib in redgrapes superglue quark;
    do
	truncate -s 0 ${lib}_data
	for n_tasks in 1024 2048 4096 8192 16384;
	do
	    DATA=""
	    ARGS="$n_tasks $n_resources $min_dependencies $max_dependencies $min_task_duration $max_task_duration $n_workers $block_exec"

	    for i in $(seq $n_repeat);
	    do
		echo $lib $ARGS $i
		OUTPUT=$(xargs ./build/$lib <<< "$ARGS $i")

		grep -s 'success' <<< $OUTPUT >/dev/null || exit

		TOTAL=$(grep -Po 'total \K[.0-9]*' <<< $OUTPUT)
		IDEAL=$(grep -Po 'critical path \K[.0-9]*' <<< $OUTPUT)
		EMPLACE=$(grep -Po 'emplacement \K[.0-9]*' <<< $OUTPUT)
		GAP=$(grep -Po 'scheduling gap \K[.0-9]*' <<< $OUTPUT)
		DIFF=$(bc -l <<< "($TOTAL - $IDEAL) / $n_tasks")

		echo "gap = " $GAP
		echo "emp = " $EMPLACE
		
		DATA="$DIFF $DATA"
	    done
	    
	    echo $DATA
	    MIN=$(awk '{min = $1; for(i=1;i<=NF;i++) { if($i < min) { min = $i } } } END { printf "%f\n", min }' <<< $DATA)
	    MAX=$(awk '{max = $1; for(i=1;i<=NF;i++) { if($i > max) { max = $i } } } END { printf "%f\n", max }' <<< $DATA)
	    AVG=$(awk '{sum = 0; for(i=1;i<=NF;i++) { sum += $i } } END { printf "%f\n", sum/NF }' <<< $DATA)
	    VAR=$(awk "{ varsum = 0; for(i=1;i<=NF;i++) { varsum += (\$i-$AVG)*(\$i-$AVG); } } END { printf \"%f\n\", varsum/NF }" <<< $DATA)
	    SIG=$(bc -l <<< "sqrt($VAR)")

	    echo "min=$MIN, max=$MAX, avg=$AVG, sigma=$SIG"
	    echo "$n_tasks $AVG $MIN $MAX $SIG" >> ${lib}_data
	done
    done
}

plot()
{
    OUTPUT="bench_res${n_resources}_dep${min_dependencies}_${max_dependencies}_dur${min_task_duration}_${max_task_duration}_thr${n_workers}.png"
    TITLE="$n_resources resources,\n$min_dependencies - $max_dependencies dependencies per task,\n $min_task_duration - $max_task_duration μs task duration,\n $n_workers threads\nHost: $(cat /etc/hostname)"
    LABEL_X="#tasks"
    LABEL_Y="avg runtime overhead per task (μs)"
    LOGX=1

    . ../plot.sh <<< "quark_data Quark #86C4FF #006DD5
superglue_data SuperGlue #88F176 #20D500
redgrapes_data RedGrapes #C976F1 #670496"
}

build
run
plot

