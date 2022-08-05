#!/bin/sh

mkdir build
cd build
cmake ..
make -j
cd ..

n_repeat=15
n_workers=4
matrix_size=4096

for lib in redgrapes superglue quark;
do
    truncate -s 0 ${lib}_data
    for nblks in 4 8 16 32 64;
    do
	DATA=""

        blksz=$((matrix_size / nblks))

	for i in $(seq $n_repeat);
	do
	    echo $lib $blksz $nblks $n_workers
	    OUTPUT=$(numactl -C 0-$(( n_workers )) ./build/$lib $blksz $nblks $n_workers)
	    TOTAL=$(echo $OUTPUT | grep -Po 'total \K[0-9.]*')
	    DATA="$TOTAL $DATA"
	done

	echo $DATA
	MIN=$(awk '{min = $1; for(i=1;i<=NF;i++) { if($i < min) { min = $i } } } END { printf "%f\n", min }' <<< $DATA)
	MAX=$(awk '{max = $1; for(i=1;i<=NF;i++) { if($i > max) { max = $i } } } END { printf "%f\n", max }' <<< $DATA)
	AVG=$(awk '{sum = 0; for(i=1;i<=NF;i++) { sum += $i } } END { printf "%f\n", sum/NF }' <<< $DATA)
	VAR=$(awk "{ varsum = 0; for(i=1;i<=NF;i++) { varsum += (\$i-$AVG)*(\$i-$AVG); } } END { printf \"%f\n\", varsum/NF }" <<< $DATA)
	SIG=$(bc -l <<< "sqrt($VAR)")

	echo "min=$MIN, max=$MAX, avg=$AVG, sigma=$SIG"
	echo "$nblks $AVG $MIN $MAX $SIG" >> ${lib}_data
    done
done

OUTPUT="bench_cholesky_w${n_workers}_m${matrix_size}.png"

gnuplot -p \
   -e "set output \"${OUTPUT}\"" \
   -e 'set terminal pngcairo enhanced truecolor size 1200,900 font "DejaVu Sans,24"' \
   -e "set title \"cholesky factorization\n $matrix_size x $matrix_size matrix of 64-bit float  \n $n_workers workers\"" \
   -e 'set xlabel "tiling factor (#tiles^{0.5})"' \
   -e 'set ylabel "runtime (ms)"' \
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
