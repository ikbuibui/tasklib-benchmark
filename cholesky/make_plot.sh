#!/bin/sh

mkdir build
cd build
cmake ..
make -j
cd ..

n_repeat=5
n_workers=4
delay=10

for lib in redgrapes superglue quark;
do
    truncate -s 0 ${lib}_data
    for dim in 4 8 16 32 64;
    do
	DATA=""

	for i in $(seq $n_repeat);
	do
	    echo $lib $dim $delay $n_workers
	    OUTPUT=$(numactl -C 0-$((n_workers - 1)) ./build/$lib $dim $delay $n_workers)
	    TOTAL=$(echo $OUTPUT | grep -Po 'total \K[0-9.]*')
	    DATA="$TOTAL $DATA"
	done

	echo $DATA
	MIN=$(awk '{min = $1; for(i=1;i<=NF;i++) { if($i < min) { min = $i } } } END { printf "%f\n", min }' <<< $DATA)
	MAX=$(awk '{max = $1; for(i=1;i<=NF;i++) { if($i > max) { max = $i } } } END { printf "%f\n", max }' <<< $DATA)
	AVG=$(awk '{sum = 0; for(i=1;i<=NF;i++) { sum += $i } } END { printf "%f\n", sum/NF }' <<< $DATA)

	echo "min=$MIN, max=$MAX, avg=$AVG"
	echo "$dim $AVG $MIN $MAX" >> ${lib}_data
    done
done

OUTPUT="bench_cholesky_w${n_workers}_d${delay}.png"

gnuplot -p \
   -e "set output \"${OUTPUT}\"" \
   -e 'set terminal png size 800,600 enhanced font "Computer Modern,16"' \
   -e "set title \"cholesky factorization\n $delay μs task duration,\n $n_workers workers\"" \
   -e 'set xlabel "matrix size (#tiles^{0.5})"' \
   -e 'set ylabel "runtime (ms)"' \
   -e 'set key right top' \
   -e 'set grid' \
   -e 'set logscale x 2' \
   -e 'set logscale y 2' \
   -e 'plot "redgrapes_data" using 1:2:3:4 title "RedGrapes" with yerrorlines,
            "superglue_data" using 1:2:3:4 title "SuperGlue" with yerrorlines,
            "quark_data" using 1:2:3:4 title "Quark" with yerrorlines'

