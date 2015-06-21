set terminal postscript eps size 5,2 enhanced color 
set output '| ps2pdf - cachingStrat_evaluation.pdf'
set title "Finding the optimal number of occurences before deploying to the POP"
set grid
set xrange [0:18]
set yrange [0:30]
set xlabel "# of occurence before caching"
set ylabel "% of SLA violations"

stats "eval2.csv" u 1:($2/12) nooutput
set label 2 "    Minimun" at STATS_pos_min_y, STATS_min_y offset 0.5,-1
plot  'eval2.csv' using 1:($2/12) with lines  notitle lc rgb "blue",\
STATS_min_y w l lc rgb"#00ffff" notitle

