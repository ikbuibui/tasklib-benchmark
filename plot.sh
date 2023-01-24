#!/bin/sh

GNUPLOT_CMD=""

while IFS=' ' read -r FILE NAME COLOR1 COLOR2;
do
    GNUPLOT_CMD="$GNUPLOT_CMD
\"$FILE\" using 1:(\$2-\$5):(\$2+\$5) with filledcurves fs transparent solid 0.4 lc rgb \"$COLOR1\" title \"\",
\"$FILE\" using 1:2 title \"\" with lines dt 3 lc rgb \"$COLOR2\",
\"$FILE\" using 1:2:3:4 title \"$NAME\" with yerrorbars lt 5 lc rgb \"$COLOR2\","
done

GNUPLOT_CMD=$(sed '$s/,$//' <<< $GNUPLOT_CMD | tail -n+1)
GNUPLOT_E=$"set output \"${OUTPUT}\"\n
set terminal pngcairo enhanced truecolor size 1200,900 font \"DejaVu Sans,24\"\n
set title \"${TITLE}\"\n
set xlabel \"${LABEL_X}\"\n
set ylabel \"${LABEL_Y}\"\n
set key right top\n
set grid\n
set style line 11 lc rgb \"#555555\" lt 1\n
set border 3 back ls 11\n
set tics nomirror\n
set style line 12 lc rgb \"#555555\" lt 0 lw 1\n
set grid back ls 12\n
plot ${GNUPLOT_CMD}"

[ -e $LOGX ] || GNUPLOT_E="set logscale x 2\n$GNUPLOT_E"

echo -e $GNUPLOT_E
echo -e $GNUPLOT_E | gnuplot


