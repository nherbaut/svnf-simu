
#set terminal qt size 800,600
set terminal postscript color
set output '| ps2pdf - CP+POP_evaluation.pdf'
samples(x) = $0 > 4 ? 5 : ($0+1)
avg5_1(x) = (shift5_1(x), (back1_1+back2_1+back3_1+back4_1+back5_1)/samples($0))
avg5_2(x) = (shift5_2(x), (back1_2+back2_2+back3_2+back4_2+back5_2)/samples($0))
avg5_3(x) = (shift5_3(x), (back1_3+back2_3+back3_3+back4_3+back5_3)/samples($0))
shift5_1(x) = (back5_1 = back4_1, back4_1 = back3_1, back3_1 = back2_1, back2_1 = back1_1, back1_1 = x)
shift5_2(x) = (back5_2 = back4_2, back4_2 = back3_2, back3_2 = back2_2, back2_2 = back1_2, back1_2 = x)
shift5_3(x) = (back5_3 = back4_3, back4_3 = back3_3, back3_3 = back2_3, back2_3 = back1_3, back1_3 = x)
init(x) = (back1_1 = back2_1 = back3_1 = back4_1 = back5_1 = back1_2 = back2_2 = back3_2 = back4_2 = back5_2 = back1_3 = back2_3 = back3_3 = back4_3 = back5_3 = sum = 0)

set multiplot layout 2, 2
unset grid
set grid x y

set xtics 30


#sset format y2 '%2.0f%%' 






set ytics nomirror
unset y2tics 

set tics out
datafile = "./server-loads0.csv"

set title "Requested Videos"

set ylabel "# of requests per seconds"
set xrange [0:90]
set yrange [0:25]
set y2range [0:120]
#set origin 0.0,0.5

set yrange [0:75]
set y2range [0:20]

set ylabel "# of Mbit transmitted"
set xlabel ''
set title "Scenario A: Throughput for CP network alone"
#


plot  sum=init(0),\
datafile using 1:($2/1000000)          axes x1y1 title "Throughput"          lt rgb "green"                 with lines , \
#datafile using 1:(avg5($4))         axes x1y2 title  "SLA violations"              lt rgb "red"                 with lines ,\
#datafile using 1:((sum = sum + $4)) axes x1y2 title  "SLA violations cumulated"     lt rgb "red" dashtype '.'     with lines  

#

#########
set title "Scenario B: Throughput for CP+POP";
set ylabel ''
datafile = "./server-loads1.csv"
#
plot sum=init(0), \
datafile using 1:($2/1000000)         axes x1y1 title "Throughput for CP"         lt rgb "green"     with lines        , \
datafile using 1:($3/1000000)          axes x1y1 title  "Throughput for POP"                     lt rgb "blue" with lines, \
#datafile using 1:((sum = sum + $4)) axes x1y2 title  "SLA violations cumulated"        lt rgb "red" dashtype '.' with lines,\
#set title "with POP"
#show y2range

set title "Requets from clients and SLA Violations"
set ylabel "# of requests per seconds"
set y2label "# of requests, cumulated"
set y2tics out

##############""
#unset y2tics
set xrange [0:90]
set yrange [0:25]
set y2range [0:80]
POP = "./server-loads1.csv"
CP = "./server-loads0.csv"
set origin 0, 0.0
set size 1, 0.5
set key below 
unset grid
set grid x y2
sum1=sum2=sum3=sum_CP=sum_POP=0
plot sum=init(0),\
CP using 1:(avg5_1($5))       axes x1y1     title  "Requests per seconds"            lt rgb "red" with filledcurves x1,\
CP using 1:(avg5_2($4))             axes x1y1     title  "SLA violations for scenario A"             lt rgb "green"  with filledcurves x1,\
CP using 1:((sum_CP=sum_CP+$4))             axes x1y2     title  "Cumulated SLA violations for scenario A"             lt rgb "green"  with lines,\
POP using 1:(avg5_3($4))             axes x1y1     title  "SLA violations scenario B"             lt rgb "blue"  with filledcurves x1,\
POP using 1:((sum_POP=sum_POP+$4))             axes x1y2     title  "Cumulated SLA violations for scenario B"             lt rgb "blue"     with lines 


