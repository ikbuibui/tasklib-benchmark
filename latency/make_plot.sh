#!/bin/sh

mkdir build
cd build
cmake .. -DCMAKE_CXX_FLAGS="-DDEPENDENCIES_PER_TASK=$n_dependencies_per_task"
make -j
cd ..

truncate -s 0 data

i=0

for lib in superglue quark redgrapes;
do
    OUTPUT=$(./build/$lib)
    echo "$i $lib $(echo $OUTPUT | grep -Po 'latency = \K[0-9.]*')" >> data

    i=$((i + 1))
done

OUTPUT="bench_latency.png"

gnuplot -p \
   -e "set output \"${OUTPUT}\"" \
   -e 'set terminal png size 800,600 enhanced font "Computer Modern,16"' \
   -e 'set title "latency of one single task"' \
   -e 'set ylabel "latency (μs)"' \
   -e 'set key off' \
   -e 'set boxwidth 0.5' \
   -e 'set style fill solid' \
   -e 'plot "data" using 1:3:xtic(2) with boxes'

