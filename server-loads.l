#
# $Id: multiaxis.dem,v 1.1 2007/06/09 22:10:45 sfeam Exp $
#

# Use the 3rd plot of the electronics demo to show off
# the use of multiple x and y axes in the same plot.
# 
set multiplot layout 2, 1 
set grid x y2
set key center bottom
set title "without POP"
set xlabel "time (s)"
set ylabel "# of bite transmitted"
set y2label "# of SLA violation"
set ytics nomirror
set y2tics
set tics out
set autoscale  y
set autoscale y2
plot "./build/server-loads-nonpop.csv" using 1:2 with lines axes x1y1 title "content provider", \
"./build/server-loads-nonpop.csv" using 1:4 with lines axes x2y2 title  "sla violation"
plot "./build/server-loads.csv" using 1:2 with lines axes x1y1 title "content provider load", \
"./build/server-loads.csv" using 1:3  with lines axes x1y1 title  "pop load", \
"./build/server-loads.csv" using 1:4 with lines axes x2y2 title  "sla violation"
set title "with POP"

pause -1

