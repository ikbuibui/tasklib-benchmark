#!/bin/sh


n_resources=${n_resources-20}
min_dependencies=${min_dependencies-0}
max_dependencies=${max_dependencies-5}
n_workers=${n_workers-4}
task_duration=${task_duration-25}
n_repeat=${n_repeat-10}

mkdir build
cd build
cmake ..
make -j
cd ..

for lib in redgrapes superglue quark;
do
    truncate -s 0 ${lib}_data
    for n_tasks in 1024 2048 4096 8192 16384;
    do
	DATA=""

	for i in $(seq $n_repeat);
	do
	    echo $lib $n_tasks $n_resources $min_dependencies $max_dependencies $task_duration $n_workers $i
	    OUTPUT=$(numactl -C 0-$((n_workers - 1)) ./build/$lib $n_tasks $n_resources $min_dependencies $max_dependencies $task_duration $n_workers $i)
	    TOTAL=$(echo $OUTPUT | grep -Po 'total \K[0-9]*')
	    CRITICAL_PATH_LEN=$(echo $OUTPUT | grep -Po 'max path length = \K[0-9]*')
	    echo $OUTPUT
	    IDEAL=$(bc -l <<< "$CRITICAL_PATH_LEN * $task_duration")
	    DIFF=$(bc -l <<< "($TOTAL - $IDEAL) / $n_tasks")

	    DATA="$DIFF $DATA"
	done

	echo $DATA
	MIN=$(awk '{min = $1; for(i=1;i<=NF;i++) { if($i < min) { min = $i } } } END { printf "%f\n", min }' <<< $DATA)
	MAX=$(awk '{max = $1; for(i=1;i<=NF;i++) { if($i > max) { max = $i } } } END { printf "%f\n", max }' <<< $DATA)
	AVG=$(awk '{sum = 0; for(i=1;i<=NF;i++) { sum += $i } } END { printf "%f\n", sum/NF }' <<< $DATA)

	echo "min=$MIN, max=$MAX, avg=$AVG"
	echo "$n_tasks $AVG $MIN $MAX" >> ${lib}_data
    done
done

OUTPUT="bench_res${n_resources}_dep${min_dependencies}_${max_dependencies}_dur${task_duration}_thr${n_workers}.png"

gnuplot -p \
   -e "set output \"${OUTPUT}\"" \
   -e 'set terminal png size 800,600 enhanced font "Computer Modern,16"' \
   -e "set title \"$n_resources resources,\n$min_dependencies - $max_dependencies dependencies per task,\n $task_duration μs task duration,\n $n_workers threads\"" \
   -e 'set xlabel "number of tasks"' \
   -e 'set ylabel "avg runtime overhead per task (μs)"' \
   -e 'set key right top' \
   -e 'set grid' \
   -e 'set logscale x 2' \
   -e 'plot "redgrapes_data" using 1:2:3:4 title "RedGrapes" with yerrorlines,
            "superglue_data" using 1:2:3:4 title "SuperGlue" with yerrorlines,
            "quark_data" using 1:2:3:4 title "Quark" with yerrorlines'

