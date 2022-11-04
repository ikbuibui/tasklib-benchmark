#!/bin/sh

n_repeat=${n_repeat-5}
n_workers=${n_workers-8}
matrix_size=${matrix_size-4096}

build()
{
    mkdir -p build
    cd build
    cmake ..
    make -j
    cd ..
}

run()
{
    for lib in redgrapes superglue quark;
    do
	truncate -s 0 ${lib}_data
	for nblks in 4 8 16 32 64 128;
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
}

plot()
{
    OUTPUT="bench_cholesky_w${n_workers}_m${matrix_size}.png"
    TITLE="cholesky factorization\n $matrix_size x $matrix_size matrix of 64-bit float \n $n_workers workers \n host: $(hostname)"
    LABEL_X="tiling factor (#tiles^{0.5})"
    LABEL_Y="runtime (ms)"

    . ../plot.sh <<< "quark_data Quark #86C4FF #006DD5
superglue_data SuperGlue #88F176 #20D500
redgrapes_data RedGrapes #C976F1 #670496"
}

build
run
plot


