#!/bin/sh

mkdir build
cd build
cmake ..
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
   -e 'set terminal pngcairo enhanced truecolor size 1200,900 font "DejaVu Sans,24"' \
   -e 'set title "latency of one single task"' \
   -e 'set xlabel "latency (μs)"' \
   -e 'set yrange [0:*]' \
   -e 'set style fill solid' \
   -e 'unset key' \
   -e 'myBoxWidth = 0.8' \
   -e 'set style line 11 lc rgb "#555555" lt 1' \
   -e 'set border 3 back ls 11' \
   -e 'set tics nomirror' \
   -e 'set style line 12 lc rgb "#555555" lt 0 lw 1' \
   -e 'set grid back ls 12'  \
   -e 'set offsets 0,0,0.5-myBoxWidth/2.,0.5' \
   -e 'plot "data" using 2:0:(0):2:($0-myBoxWidth/2.):($0+myBoxWidth/2.):($0+1):ytic(1) with boxxyerror lc var'

