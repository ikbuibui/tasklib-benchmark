#!/bin/sh

mkdir build
cd build
cmake .. -DCMAKE_CXX_FLAGS="-DDEPENDENCIES_PER_TASK=$n_dependencies_per_task"
make -j
cd ..

truncate -s 0 data

i=0

for lib in redgrapes superglue quark;
do
    OUTPUT=$(./build/$lib)
    echo "$lib $(echo $OUTPUT | grep -Po 'latency = \K[0-9.]*')" >> data

    i=$((i + 1))
done

OUTPUT="bench_latency.png"

gnuplot -p \
   -e "set output \"${OUTPUT}\"" \
   -e 'set terminal png size 800,600 enhanced font "Computer Modern,16"' \
   -e 'set title "latency of one single task"' \
   -e 'set xlabel "latency (μs)"' \
   -e 'set yrange [0:*]' \
   -e 'set style fill solid' \
   -e 'unset key' \
   -e 'myBoxWidth = 0.8' \
   -e 'set offsets 0,0,0.5-myBoxWidth/2.,0.5' \
   -e 'plot "data" using 2:0:(0):2:($0-myBoxWidth/2.):($0+myBoxWidth/2.):($0+1):ytic(1) with boxxyerror lc var'

