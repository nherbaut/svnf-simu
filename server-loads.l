#
# $Id: multiaxis.dem,v 1.1 2007/06/09 22:10:45 sfeam Exp $
#

# Use the 3rd plot of the electronics demo to show off
# the use of multiple x and y axes in the same plot.
# 
set multiplot layout 2, 1 
set grid x y2
set xrange [0:70]
set y2range [0:0.25]
#set yrange [0:6000000]
set format y2 '%2.0f%%' 
set key center bottom
set title "without POP"
set xlabel "time (s)"
set ylabel "# of bit transmitted"
set y2label "% of SLA violation"
set ytics nomirror
set y2tics nomirror
set tics out
plot "./build/server-loads0.csv" using 1:2 with lines axes x1y1 title "content provider" lt rgb "green", \
"./build/server-loads0.csv" using 1:4 with lines axes x2y2 title  "sla violation" lt rgb "red"
set title "with POP"
plot "./build/server-loads1.csv" using 1:2 with lines axes x1y1 title "content provider load" lt rgb "green", \
"./build/server-loads1.csv" using 1:3  with lines axes x1y1 title  "pop load" lt rgb "blue", \
"./build/server-loads1.csv" using 1:4 with lines axes x2y2 title  "sla violation" lt rgb "red"
set title "with POP"
show y2range
pause -1

