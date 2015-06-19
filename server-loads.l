
set terminal qt size 800,600

samples(x) = $0 > 4 ? 5 : ($0+1)
avg5(x) = (shift5(x), (back1+back2+back3+back4+back5)/samples($0))
shift5(x) = (back5 = back4, back4 = back3, back3 = back2, back2 = back1, back1 = x)
init(x) = (back1 = back2 = back3 = back4 = back5 = sum = 0)

set multiplot layout 2, 2
set grid x y2
#set xrange [60:150]
set xtics 30


#sset format y2 '%2.0f%%' 




set yrange [0:25]

set ytics nomirror

set tics out
datafile = "./build/server-loads0.csv"
set title "Requested Videos"

set ylabel "# of requests per seconds"
set yrange [0:50]

set origin 0.0,0.5
##############""
set key top left
plot datafile using 1:5 with lines axes x1y1 title  "Request per seconds" lt rgb "black";






###########""

set yrange [0:75]
set y2range [0:50]
set y2tics nomirror
set ylabel "# of Mbit transmitted"
set xlabel ''
set title "Throughput and SLA violation with CP network alone"
#


plot  sum=init(0),\
datafile using 1:($2/1000000) with lines axes x1y1 title "content provider" lt rgb "green", \
datafile using 1:(avg5($4)) with lines axes x1y2 title  "sla violation" lt rgb "red"
#

#########
set title "Throughput and SLA violation with a POP";

datafile = "./build/server-loads1.csv"
set size 1, 0.5
#
plot sum=init(0), \
datafile using 1:($2/1000000) with lines axes x1y1 title "content provider load" lt rgb "green", \
datafile using 1:($3/1000000)  with lines axes x1y1 title  "pop load" lt rgb "blue", \
datafile using 1:(avg5($4)) with lines axes x1y2 title  "sla violation" lt rgb "red",\
#datafile using 1:5 with lines axes x1y2 title  "Request per seconds" lt rgb "yellow"
set title "with POP"
show y2range
pause -1

