#!/bin/sh

GNUPLOT_CMD=""

while IFS=' ' read -r FILE NAME COLOR1 COLOR2;
do
    GNUPLOT_CMD="$GNUPLOT_CMD
          \"$FILE\" using 1:(\$2-\$5):(\$2+\$5) with filledcurves fs transparent solid 0.4 lc rgb \"$COLOR1\" title \"\",
    	  \"$FILE\" using 1:2 title \"\" with lines dt 3 lc rgb \"$COLOR2\",
	  \"$FILE\" using 1:2:3:4 title \"$NAME\" with yerrorbars lt 5 lc rgb \"$COLOR2\","
done

GNUPLOT_CMD=$(sed '$s/,$//' <<< $GNUPLOT_CMD)

gnuplot -p \
   -e "set output \"${OUTPUT}\"" \
   -e "set terminal pngcairo enhanced truecolor size 1200,900 font \"DejaVu Sans,24\"" \
   -e "set title \"${TITLE}\"" \
   -e "set xlabel \"${LABEL_X}\"" \
   -e "set ylabel \"${LABEL_Y}\"" \
   -e 'set key right top' \
   -e 'set grid' \
   -e 'set logscale x 2' \
   -e 'set style line 11 lc rgb "#555555" lt 1' \
   -e 'set border 3 back ls 11' \
   -e 'set tics nomirror' \
   -e 'set style line 12 lc rgb "#555555" lt 0 lw 1' \
   -e 'set grid back ls 12' \
   -e "plot ${GNUPLOT_CMD}"

