set terminal png nocrop enhanced size 600,300
set style data lines
set timefmt "%Y-%m-%d %H:%M:%S"
set xdata time
set format x "%H:%M"
set grid

load "xrange.plot"

set xtics 15000

set output 'wind_speed.png'
set title "Wind m/s"
set yrange [0:]
set lmargin 8 

plot 'weatherdata.tmp' using 1:8 t '' lt 2 with dots, \
     'weatherdata.tmp' using 1:6 t '' lt 2 with dots, \
     'weatherdata.tmp' using 1:7 t '' lt -1 smooth bezier

set output 'wind_dir.png'
set title "Wind degrees"
set yrange [0:360]
set ytics ("0 N" 0, "" 45, "90 E" 90, "" 135,\
  	   "180 S" 180, "" 225, "270 W" 270, "" 315,\
	   "360 N" 360)

plot 'weatherdata.tmp' using 1:5 t '' lt 2 with dots, \
	'weatherdata.tmp' using 1:3 t '' lt 2 with dots, \
	'weatherdata.tmp' using 1:4 t '' lt -1 smooth bezier
