
set terminal qt size 800,600

samples(x) = $0 > 4 ? 5 : ($0+1)
avg5(x) = (shift5(x), (back1+back2+back3+back4+back5)/samples($0))
shift5(x) = (back5 = back4, back4 = back3, back3 = back2, back2 = back1, back1 = x)
init(x) = (back1 = back2 = back3 = back4 = back5 = sum = 0)

set multiplot layout 2, 2
set grid x y2
set xrange [0:150]
set xtics 30


#sset format y2 '%2.0f%%' 






set ytics nomirror

set tics out
datafile = "./build/server-loads0.csv"
set title "Requested Videos"

set ylabel "# of requests per seconds"

set yrange [0:30	]
set y2range [0:600]
set origin 0.0,0.5




##############""
set key top left
plot sum=init(0),datafile using 1:5   axes x1y1 title  "Request per seconds" with lines lt rgb "black"  , \
#datafile using 1:((sum = sum + $5))   axes x1y1  title  "cumulated Requests" lt rgb "black"  dashtype '..' with lines,

set linestyle  1 lt 1 lc 7 # black-solid




###########""

set yrange [0:75]

set y2tics nomirror
set ylabel "# of Mbit transmitted"
set xlabel ''
set title "Throughput and SLA violation with CP network alone"
#


plot  sum=init(0),\
datafile using 1:($2/1000000)  		axes x1y1 title "content provider"  		lt rgb "green" 				with lines , \
datafile using 1:(avg5($4)) 		axes x1y2 title  "sla violation"  			lt rgb "red" 				with lines ,\
datafile using 1:((sum = sum + $4)) axes x1y2 title  "sla violation cumulated" 	lt rgb "red" dashtype '.' 	with lines  

#

#########
set title "Throughput and SLA violation with a POP";

datafile = "./build/server-loads1.csv"
set size 1, 0.5
#
plot sum=init(0), \
datafile using 1:($2/1000000) 		axes x1y1 title "content provider load" 		lt rgb "green" 	with lines		, \
datafile using 1:($3/1000000)  		axes x1y1 title  "pop load" 					lt rgb "blue" with lines, \
datafile using 1:(avg5($4)) 		axes x1y2 title  "sla violation" 				lt rgb "red" with lines,\
datafile using 1:((sum = sum + $4)) axes x1y2 title  "sla violation cumulated"		lt rgb "red" dashtype '.' with lines,\
#set title "with POP"
#show y2range
pause -1

