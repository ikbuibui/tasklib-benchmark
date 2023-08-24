#!/bin/sh

cd build
make -j redgrapes
cd ..

mkdir -p tmptraceimg

set -e
trap "exit 1" ERR

run_rg() {
    for n_workers in 8 16 32 64;
    do
	echo "run redgrapes $n_tasks $n_resources $min_dependencies $max_dependencies $min_task_duration $max_task_duration $n_workers $blocked"
	./build/redgrapes $n_tasks $n_resources $min_dependencies $max_dependencies $min_task_duration $max_task_duration $n_workers $blocked 2>&1 | grep 'success' || (echo "A B O R T" && exit 1)
	inkscape trace_redgrapes.svg -o tmptraceimg/trace_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_${min_task_duration}_${max_task_duration}_${n_workers}_${blocked}_$(uname -n).png || exit 1
	rm trace_redgrapes.svg || exit 1
    done

    mogrify -crop 30%x100%+0+0 tmptraceimg/trace_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_${min_task_duration}_${max_task_duration}_8_${blocked}_$(uname -n).png
    mogrify -crop 60%x100%+0+0 tmptraceimg/trace_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_${min_task_duration}_${max_task_duration}_16_${blocked}_$(uname -n).png

    montage "tmptraceimg/trace_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_${min_task_duration}_${max_task_duration}_8_${blocked}_$(uname -n).png" \
	    "tmptraceimg/trace_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_${min_task_duration}_${max_task_duration}_16_${blocked}_$(uname -n).png" \
	    "tmptraceimg/trace_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_${min_task_duration}_${max_task_duration}_32_${blocked}_$(uname -n).png" \
	    "tmptraceimg/trace_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_${min_task_duration}_${max_task_duration}_64_${blocked}_$(uname -n).png" \
	    -font "Utopia" \
	    -border 10 -tile 1x4 -geometry +1+1 "tmptraceimg/s1_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_${min_task_duration}_${max_task_duration}_${blocked}_$(uname -n).png"
}

n_tasks=512
n_resources=64
blocked=true
min_dependencies=1
max_dependencies=1

min_task_duration=50
max_task_duration=50
run_rg

min_task_duration=50
max_task_duration=150
run_rg

min_task_duration=100
max_task_duration=200
run_rg

min_task_duration=200
max_task_duration=200
run_rg

montage -label "50μs-50μs" "tmptraceimg/s1_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_50_50_${blocked}_$(uname -n).png" \
	-label "50μs-150μs" "tmptraceimg/s1_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_50_150_${blocked}_$(uname -n).png" \
	-label "100μs-200μs" "tmptraceimg/s1_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_100_200_${blocked}_$(uname -n).png" \
	-label "200μs-200μs" "tmptraceimg/s1_${n_tasks}_${n_resources}_${min_dependencies}_${max_dependencies}_200_200_${blocked}_$(uname -n).png" \
	-font "Utopia" -pointsize 60 \
	-border 45 -tile 4x1 -geometry +1+1 "tmptraceimg/s1_$(uname -n).png"


