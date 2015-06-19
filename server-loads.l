#
# $Id: multiaxis.dem,v 1.1 2007/06/09 22:10:45 sfeam Exp $
#

# Use the 3rd plot of the electronics demo to show off
# the use of multiple x and y axes in the same plot.
# 


samples(x) = $0 > 4 ? 5 : ($0+1)
avg5(x) = (shift5(x), (back1+back2+back3+back4+back5)/samples($0))
shift5(x) = (back5 = back4, back4 = back3, back3 = back2, back2 = back1, back1 = x)
init(x) = (back1 = back2 = back3 = back4 = back5 = sum = 0)

set multiplot layout 2, 1 
set grid x y2
#set xrange [0:200]
set y2range [0:50]
#set yrange [0:20000000]
#sset format y2 '%2.0f%%' 
set key off
set title "without POP"
set xlabel "time (s)"
set ylabel "# of bit transmitted"
set y2label "# of SLA violation per s"
set ytics nomirror
set y2tics nomirror
set tics out
datafile = "./build/server-loads0.csv"
plot  sum=init(0),\
datafile using 1:2 with lines axes x1y1 title "content provider" lt rgb "green", \
datafile using 1:(avg5($4)) with lines axes x2y2 title  "sla violation" lt rgb "red"
#"./build/server-loads0.csv" using 1:5 with lines axes x2y2 title  "Total Video Asked" lt rgb "orange"
set title "with POP"
set key above
datafile = "./build/server-loads1.csv"
plot sum=init(0), \
	datafile using 1:2 with lines axes x1y1 title "content provider load" lt rgb "green", \
datafile using 1:3  with lines axes x1y1 title  "pop load" lt rgb "blue", \
datafile using 1:(avg5($4)) with lines axes x2y2 title  "sla violation" lt rgb "red"


#"./build/server-loads1.csv" using 1:5 with lines axes x2y2 title  "Total Video Asked" lt rgb "orange"
set title "with POP"
show y2range
pause -1

