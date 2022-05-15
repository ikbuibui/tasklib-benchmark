#!/bin/sh

n_resources=10
min_dependencies=0
max_dependencies=4
n_threads=4
task_duration=1

n_repeat=5

mkdir build
cd build
cmake .. -DCMAKE_CXX_FLAGS="-DDEPENDENCIES_PER_TASK=$n_dependencies_per_task"
make -j
cd ..

for lib in redgrapes superglue quark;
do
    truncate -s 0 ${lib}_data
    for n_tasks in 1500 3000 7000 10000;
    do
	DATA=""

	for i in $(seq $n_repeat);
	do
	    echo $lib $n_tasks $n_resources $min_dependencies $max_dependencies $task_duration $n_threads $i
	    TOTAL=$(numactl -C 1-$n_threads ./build/$lib $n_tasks $n_resources $min_dependencies $max_dependencies $task_duration $n_threads $i | grep -Po '\K[0-9]*')
	    IDEAL=$(bc -l <<< "($task_duration * $n_tasks) / $n_threads")
	    DIFF=$(bc -l <<< "($TOTAL/1000) - $IDEAL")

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

gnuplot -p \
   -e "set title \"$n_resources resources, dependencies per task: $min_dependencies - $max_dependencies, $task_duration μs task duration, $n_threads threads\"" \
   -e 'set xlabel "number of tasks"' \
   -e 'set ylabel "runtime overhead (μs)"' \
   -e 'set key right bottom' \
   -e 'set grid' \
   -e 'plot "redgrapes_data" using 1:2:3:4 title "RedGrapes" with yerrorlines,
            "superglue_data" using 1:2:3:4 title "SuperGlue" with yerrorlines,
            "quark_data" using 1:2:3:4 title "Quark" with yerrorlines'

#-e 'set logscale x 2' \
   #-e 'set yrange[0:1]' \

