#!/bin/sh

n_tasks=8192
n_resources=10
n_workers=2
n_repeat=${n_repeat-15}

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
    mkdir -p data/emplacement

    for lib in quark redgrapes superglue;
    do
	truncate -s 0 data/emplacement/${lib}
	for n_deps in 0 1 2 3 4 5;
	do
	    DATA=""
	    ARGS="$n_tasks $n_resources $n_deps $n_deps 0 0 $n_workers true"

	    for i in $(seq $n_repeat);
	    do
		echo $lib $ARGS $i
		OUTPUT=$(xargs ./build/$lib <<< "$ARGS $i")

		grep -s 'success' <<< $OUTPUT >/dev/null || exit

		EMPLACE=$(grep -Po 'emplacement \K[.0-9]*' <<< $OUTPUT)
		EPT=$(bc -l <<< "$EMPLACE / $n_tasks")

		echo "emp = " $EMPLACE

		DATA="$EPT $DATA"
	    done
	    
	    echo $DATA
	    MIN=$(awk '{min = $1; for(i=1;i<=NF;i++) { if($i < min) { min = $i } } } END { printf "%f\n", min }' <<< $DATA)
	    MAX=$(awk '{max = $1; for(i=1;i<=NF;i++) { if($i > max) { max = $i } } } END { printf "%f\n", max }' <<< $DATA)
	    AVG=$(awk '{sum = 0; for(i=1;i<=NF;i++) { sum += $i } } END { printf "%f\n", sum/NF }' <<< $DATA)
	    VAR=$(awk "{ varsum = 0; for(i=1;i<=NF;i++) { varsum += (\$i-$AVG)*(\$i-$AVG); } } END { printf \"%f\n\", varsum/NF }" <<< $DATA)
	    SIG=$(bc -l <<< "sqrt($VAR)")

	    echo "min=$MIN, max=$MAX, avg=$AVG, sigma=$SIG"
	    echo "$n_deps $AVG $MIN $MAX $SIG" >> data/emplacement/${lib}
	done
    done
}

plot()
{
    mkdir -p plots
    OUTPUT="plots/emplacement.png"
    TITLE="emplacement time\nHost: $(cat /etc/hostname)"
    LABEL_X="#dependencies"
    LABEL_Y="emplacement overhead per task (μs)"

    . ../plot.sh <<< "data/emplacement/quark Quark #86C4FF #006DD5
data/emplacement/superglue SuperGlue #88F176 #20D500
data/emplacement/redgrapes RedGrapes #C976F1 #670496"
}

build
run
plot


