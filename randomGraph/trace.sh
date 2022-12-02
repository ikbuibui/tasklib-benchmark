#!/bin/sh

n_tasks=256
n_resources=8
n_workers=$n_resources
min_dependencies=1
max_dependencies=1
min_task_duration=5
max_task_duration=5
blocked=true

./build/redgrapes $n_tasks $n_resources 1 1 $min_task_duration $max_task_duration $n_workers $blocked
./build/superglue $n_tasks $n_resources 1 1 $min_task_duration $max_task_duration $n_workers $blocked
./build/quark $n_tasks $n_resources 1 1 $min_task_duration $max_task_duration $n_workers $blocked

inkscape trace_redgrapes.svg -o trace_redgrapes.png
inkscape trace_superglue.svg -o trace_superglue.png
inkscape trace_quark.svg -o trace_quark.png

convert trace_redgrapes.png -border 40x40  -gravity Northwest -pointsize 30 -annotate +5+5 'RedGrapes' trace_redgrapes.png
convert trace_superglue.png -border 40x40 -gravity Northwest -pointsize 30 -annotate +5+5 'SuperGlue' trace_superglue.png
convert trace_quark.png -border 40x40 -gravity Northwest -pointsize 30 -annotate +5+5 'Quark' trace_quark.png

montage trace_redgrapes.png trace_superglue.png trace_quark.png -tile 1x3 -geometry +1+1 trace.png
