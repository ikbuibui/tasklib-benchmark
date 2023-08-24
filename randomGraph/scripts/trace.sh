#!/bin/sh

n_tasks=128
n_resources=64
n_workers=$n_resources
min_dependencies=1
max_dependencies=1
min_task_duration=100
max_task_duration=100
blocked=true

OUTPUT_DIR=traceimg

cd build
make -j
cd ..

sleep 5
RG="$(./build/redgrapes $n_tasks $n_resources $min_dependencies $max_dependencies $min_task_duration $max_task_duration $n_workers $blocked)"

sleep 5
QK="$(./build/quark $n_tasks $n_resources $min_dependencies $max_dependencies $min_task_duration $max_task_duration $n_workers $blocked)"

sleep 5
SG="$(./build/superglue $n_tasks $n_resources $min_dependencies $max_dependencies $min_task_duration $max_task_duration $n_workers $blocked)"

echo -e "$RG" > ${OUTPUT_DIR}/rglog

echo "Redgrapes: $RG"
echo ""
echo "Superglue: $SG"
echo ""
echo "Quark: $QK"
echo ""

RG_GAP="$(echo $RG | grep -Po 'gap \K[\-0-9.]* μs')"
SG_GAP="$(echo $SG | grep -Po 'gap \K[\-0-9.]* μs')"
QK_GAP="$(echo $QK | grep -Po 'gap \K[\-0-9.]* μs')"

mkdir -p $OUTPUT_DIR
inkscape trace_redgrapes.svg -o ${OUTPUT_DIR}/trace_redgrapes.png
inkscape trace_superglue.svg -o ${OUTPUT_DIR}/trace_superglue.png
inkscape trace_quark.svg -o ${OUTPUT_DIR}/trace_quark.png

montage -label "RedGrapes (avg gap: $RG_GAP)" ${OUTPUT_DIR}/trace_redgrapes.png \
	-label "SuperGlue (avg gap: $SG_GAP)" ${OUTPUT_DIR}/trace_superglue.png \
	-label "Quark (avg gap: $QK_GAP)" ${OUTPUT_DIR}/trace_quark.png \
        -font "Utopia" -pointsize 45 \
	-border 45 -tile 1x3 -geometry +1+1 ${OUTPUT_DIR}/trace_${n_resources}_chains_${min_task_duration}_$(uname -n).png

