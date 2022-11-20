#!/bin/sh

GNUPLOT_CMD=""

while IFS=' ' read -r FILE NAME COLOR1 COLOR2;
do
    GNUPLOT_CMD="$GNUPLOT_CMD\"$FILE\" using 1:(\$2-\$5):(\$2+\$5) with filledcurves fs transparent solid 0.4 lc rgb \"$COLOR1\" title \"\",\"$FILE\" using 1:2 title \"\" with lines dt 3 lc rgb \"$COLOR2\",\"$FILE\" using 1:2:3:4 title \"$NAME\" with yerrorbars lt 5 lc rgb \"$COLOR2\","
done

GNUPLOT_CMD=$(sed '$s/,$//' <<< $GNUPLOT_CMD)
GNUPLOT_E="set output \"${OUTPUT}\"
set terminal pngcairo enhanced truecolor size 1200,900 font \"DejaVu Sans,24\"
set title \"${TITLE}\"
set xlabel \"${LABEL_X}\"
set ylabel \"${LABEL_Y}\"
set key right top
set grid
set style line 11 lc rgb \"#555555\" lt 1
set border 3 back ls 11
set tics nomirror
set style line 12 lc rgb \"#555555\" lt 0 lw 1
set grid back ls 12
plot ${GNUPLOT_CMD}"

[ -e $LOGX ] || GNUPLOT_E="set logscale x 2
$GNUPLOT_E"

echo $GNUPLOT_E

gnuplot -p <<< $GNUPLOT_E
