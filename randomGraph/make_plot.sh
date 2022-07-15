#!/bin/sh

n_resources=${n_resources-20}
min_dependencies=${min_dependencies-0}
max_dependencies=${max_dependencies-5}
min_task_duration=${min_task_duration-25}
max_task_duration=${max_task_duration-25}
n_workers=${n_workers-4}
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
	    echo $lib $n_tasks $n_resources $min_dependencies $max_dependencies $min_task_duration $max_task_duration $n_workers $i
	    OUTPUT=$(numactl -C 0-$((n_workers)) ./build/$lib $n_tasks $n_resources $min_dependencies $max_dependencies $min_task_duration $max_task_duration $n_workers $i)
	    TOTAL=$(echo $OUTPUT | grep -Po 'total \K[0-9]*')
	    IDEAL=$(echo $OUTPUT | grep -Po 'critical path \K[0-9]*')
	    echo $OUTPUT
	    DIFF=$(bc -l <<< "($TOTAL - $IDEAL) / $n_tasks")

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

OUTPUT="bench_res${n_resources}_dep${min_dependencies}_${max_dependencies}_dur${min_task_duration}_${max_task_duration}_thr${n_workers}.png"

gnuplot -p \
   -e "set output \"${OUTPUT}\"" \
   -e 'set terminal pngcairo enhanced truecolor size 1200,900 font "DejaVu Sans,24"' \
   -e "set title \"$n_resources resources,\n$min_dependencies - $max_dependencies dependencies per task,\n $min_task_duration - $max_task_duration μs task duration,\n $n_workers threads\"" \
   -e 'set xlabel "number of tasks"' \
   -e 'set ylabel "avg runtime overhead per task (μs)"' \
   -e 'set key right top' \
   -e 'set grid' \
   -e 'set logscale x 2' \
   -e 'set style line 11 lc rgb "#555555" lt 1' \
   -e 'set border 3 back ls 11' \
   -e 'set tics nomirror' \
   -e 'set style line 12 lc rgb "#555555" lt 0 lw 1' \
   -e 'set grid back ls 12'  \
   -e 'plot
            "quark_data"     using 1:($2-$5):($2+$5) with filledcurves fs transparent solid 0.4 lc rgb "#86C4FF" title "",
            "superglue_data" using 1:($2-$5):($2+$5) with filledcurves fs transparent solid 0.4 lc rgb "#88F176" title "",
	    "redgrapes_data" using 1:($2-$5):($2+$5) with filledcurves fs transparent solid 0.4 lc rgb "#C976F1" title "",

            "quark_data"     using 1:2 title "" with lines dt 3 lc rgb "#006DD5",
            "superglue_data" using 1:2 title "" with lines dt 4 lc rgb "#20D500",
            "redgrapes_data" using 1:2 title "" with lines dt 7 lc rgb "#670496",

            "quark_data"     using 1:2:3:4 title "Quark"     with yerrorbars lt 5 lc rgb "#006DD5",
            "superglue_data" using 1:2:3:4 title "SuperGlue" with yerrorbars lt 7 lc rgb "#20D500",
            "redgrapes_data" using 1:2:3:4 title "RedGrapes" with yerrorbars lt 9 lc rgb "#670496"'
