
set terminal qt size 800,600

samples(x) = $0 > 4 ? 5 : ($0+1)
avg5_1(x) = (shift5_1(x), (back1_1+back2_1+back3_1+back4_1+back5_1)/samples($0))
avg5_2(x) = (shift5_2(x), (back1_2+back2_2+back3_2+back4_2+back5_2)/samples($0))
avg5_3(x) = (shift5_3(x), (back1_3+back2_3+back3_3+back4_3+back5_3)/samples($0))
shift5_1(x) = (back5_1 = back4_1, back4_1 = back3_1, back3_1 = back2_1, back2_1 = back1_1, back1_1 = x)
shift5_2(x) = (back5_2 = back4_2, back4_2 = back3_2, back3_2 = back2_2, back2_2 = back1_2, back1_2 = x)
shift5_3(x) = (back5_3 = back4_3, back4_3 = back3_3, back3_3 = back2_3, back2_3 = back1_3, back1_3 = x)
init(x) = (back1_1 = back2_1 = back3_1 = back4_1 = back5_1 = back1_2 = back2_2 = back3_2 = back4_2 = back5_2 = back1_3 = back2_3 = back3_3 = back4_3 = back5_3 = sum = 0)

set multiplot layout 2, 2
set grid x y2
set xrange [0:300]
set xtics 30


#sset format y2 '%2.0f%%' 






set ytics nomirror
set y2tics nomirror

set tics out
datafile = "./build/server-loads0.csv"

set title "Requested Videos"

set ylabel "# of requests per seconds"

set yrange [0:25]
set y2range [0:120]
#set origin 0.0,0.5

set yrange [0:75]
set y2range [0:20]

set ylabel "# of Mbit transmitted"
set xlabel ''
set title "Throughput and SLA violation with CP network alone"
#


plot  sum=init(0),\
datafile using 1:($2/1000000)          axes x1y1 title "content provider"          lt rgb "green"                 with lines , \
#datafile using 1:(avg5($4))         axes x1y2 title  "sla violation"              lt rgb "red"                 with lines ,\
#datafile using 1:((sum = sum + $4)) axes x1y2 title  "sla violation cumulated"     lt rgb "red" dashtype '.'     with lines  

#

#########
set title "Throughput and SLA violation with a POP 1";

datafile = "./build/server-loads1.csv"
#
plot sum=init(0), \
datafile using 1:($2/1000000)         axes x1y1 title "content provider load"         lt rgb "green"     with lines        , \
datafile using 1:($3/1000000)          axes x1y1 title  "pop load"                     lt rgb "blue" with lines, \
#datafile using 1:((sum = sum + $4)) axes x1y2 title  "sla violation cumulated"        lt rgb "red" dashtype '.' with lines,\
#set title "with POP"
#show y2range



##############""

set yrange [0:15]
set y2range [0:700]
pop = "./build/server-loads1.csv"
cp = "./build/server-loads0.csv"
set origin 0, 0.0
set size 1, 0.5
set key top left
sum1=sum2=sum3=0
plot sum=init(0),pop using 1:(avg5_3($4))             axes x1y1     title  "sla violation for POP"             lt rgb "blue"  with lines,\
cp using 1:(avg5_2($4))             axes x1y1     title  "sla violation without CP"             lt rgb "green"  with lines,\
cp using 1:(avg5_1($5))       axes x1y1     title  "Request per seconds"            lt rgb "red" with lines
pause -1

