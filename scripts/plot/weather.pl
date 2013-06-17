#!/usr/bin/perl
#
# weather.cgi
#
# plots wmr928 weather data via gnuplot to a hmtl image/jpg object
#
# html parameters are:
#	type=temp			make a temperature chart
#	res=min5,min10,hour1,...	defines the data resultion to plot with
#	start=<point in time>		defines the time to start the graph with
#	end=<point in time>		defines where to stop the graph
#	
#	pont in time can be:	YYYYMMDDhhmmss	at the specified time
#				<empty>		now
#				*n		n secoonds in the future
#				-n		n secoonds in the past
#	text=<sensor>			prints actual sensor data
#	pict=fc				return weather forecast picture
#
use IO::Socket;
use File::Copy;
#
# Set font path
#
$ENV{'GDFONTPATH'} = '/usr/local/lib/fonts';
$myfont="font 'lsans' 7";
#
# get parms
if ($ENV{'REQUEST_METHOD'} eq "GET") { $buffer = $ENV{'QUERY_STRING'}; }
else { read(STDIN, $buffer, $ENV{'CONTENT_LENGTH'}); }
#
#
$path="/var/log/wmr928";
#
# pre set values
$epochtime=time();
($second, $minute, $hour, $dayOfMonth, $month, $yearOffset, $dayOfWeek, $dayOfYear, $dayLightSavings) = gmtime($epochtime);
$now = sprintf("%04d%02d%02d%02d%02d%02d", $yearOffset+1900, $month+1, $dayOfMonth, $hour, $minute, $second);
$in{start}="20060903000000";
$in{end}=$now;
$in{res}="min10";
$in{type}="temp";
$in{text}="empty";
$in{pict}="empty";
#
# get parms as name-value pairs	
@nvpairs = split(/&/,$buffer);
foreach $pair (@nvpairs) 
  { 
  ($name, $value) = split(/=/, $pair);
  $value =~ tr/+/ /;
  $value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
  $in{$name} = $value;
  }
#
if ($in{'text'} ne "empty")
{
  $socket=IO::Socket::INET->new(PeerAddr=> 'localhost', PeerPort=>5555 , Proto=> 'tcp',
Type=> SOCK_STREAM) or die "Can't talk to localhost at port 5555";
  print "Content-type: text/plain\n\n";
  while (<$socket>) {
    chomp;
    @line = split(/ +/);
    if ($line[0] eq $in{'text'})
    { print $line[1]; }
  }
  close $socket;
  exit 0
} 
if ($in{'pict'} ne "empty")
{
  $socket=IO::Socket::INET->new(PeerAddr=> 'localhost', PeerPort=>5555 , Proto=> 'tcp',
Type=> SOCK_STREAM) or die "Can't talk to localhost at port 5555";
  print "Content-type: image/png\n\n";
  $fn="NA.png";
  while (<$socket>) {
    chomp;
    @line = split(/ +/);
    if ($line[0] eq "thb0-fc")
    { $fn = "" . ($line[1]*1) . ".png"; }
  }
  open (PIC,$fn) or die "Can't open picture file $fn.";
  print $buffer while
    (read (PIC,$buffer,65536));
  close (PIC);
  close $socket;
  exit 0
} 

if ((rindex($in{'start'},"*") >= 0) || (rindex($in{'start'},"-") >= 0))
  { 
    ($second, $minute, $hour, $dayOfMonth, $month, $yearOffset, 
	$dayOfWeek, $dayOfYear, $dayLightSavings) = gmtime($epochtime + $in{start});
    $in{start} = sprintf("%04d%02d%02d%02d%02d%02d", 
	$yearOffset+1900, $month+1, $dayOfMonth, $hour, $minute, $second);
  }
if ((rindex($in{'end'},"*") >= 0) || (rindex($in{'end'},"-") >= 0))
  { 
    ($second, $minute, $hour, $dayOfMonth, $month, $yearOffset, 
	$dayOfWeek, $dayOfYear, $dayLightSavings) = gmtime($epochtime + substr($in{end},1));
    $in{end} = sprintf("%04d%02d%02d%02d%02d%02d", 
	$yearOffset+1900, $month+1, $dayOfMonth, $hour, $minute, $second);
  }
# 
#
# generate png
print "Content-type: image/png\n\n";
open (GP, "|/usr/local/bin/gnuplot -persist") or die "no gnuplot";
# force buffer to flush after each write
use FileHandle;
GP->autoflush(1);
# standard header
print GP <<EOM;
set output
set xdata time
set ticslevel 0
#set ticscale 2 1
set tics scale 2
set mxtics default
set mytics 2
set mztics default
set mx2tics default
set my2tics default
set mcbtics default
set xtics border mirror norotate autofreq
set ytics border mirror norotate autofreq 
set ztics border nomirror norotate autofreq 
set nox2tics
set noy2tics
set grid xtics nomxtics ytics nomytics noztics nomztics nox2tics nomx2tics noy2tics nomy2tics nocbtics nomcbtics
set grid layerdefault
set cbtics border mirror norotate autofreq 
set timefmt x "%Y%m%d%H%M%S"
EOM
print GP "cd '" . $path ."'\n";
print GP "set terminal png $myfont nocrop size 610, 300\n";
print GP "set format x \"%H:%M\\n%a\\n\%d.%b\"\n";
if ($in{res} eq "day1")
{ print GP "set format x \"%a\\n\%d.%b\"\n"; }
if ($in{res} eq "month1")
{ print GP "set format x \"%b\\n%Y\"\n";
  print GP "set mxtics 2\n";
  print GP "set grid mxtics\n";
  }
#
# DAYS
if ($in{'type'} eq "days")
{
print GP "set title \"Special Days from $in{'start'} to $in{'end'}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%g days\"\n";
print GP "set yrange [0:30]\n";
print GP "set mytics 5\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "set boxwidth 0.7 relative\n";
print GP "set style fill solid 1.0\n";
print GP "plot 'th0-$in{res}' using 1:(\$11+\$12+\$13+\$14+\$15+\$16) with boxes title \"tropical nights (>>20°C)\" 1, 'th0-$in{res}' using 1:(\$11+\$12+\$13+\$14+\$15) with boxes title \"hot days (>30°C)\" 9, 'th0-$in{res}' using 1:(\$11+\$12+\$13+\$14) with boxes title \"summer days (>25°C)\" 2, 'th0-$in{res}' using 1:(\$11+\$12+\$13) with boxes title \"cold days (<<10°C)\" 5, 'th0-$in{res}' using 1:(\$11+\$12) with boxes title \"frost days(<0°C)\" 3, 'th0-$in{res}' using 1:11 with boxes title \"ice days (<<0°C)\" 8, 'rain0-$in{res}' using 1:6 with impulses lw 5 lt 4 title \"rain days\" \n";
}
#
# TEMP
if ($in{'type'} eq "temp")
{
print GP "set title \"Line Chart for Temperature from $in{'start'} to $in{'end'}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%.1f \260C\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'th0-$in{res}' using 1:2 with lines title \"temp\", 'th0-$in{res}' using 1:5 with lines title \"dew point\", 'wind0-$in{res}' using 1:8 with lines title \"wind chill\"\n";
};
#
# TEMP MIN/MAX
if ($in{'type'} eq "temp-minmax")
{
print GP "set title \"Min/Max Chart for Temperature from $in{'start'} to $in{'end'}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%.1f \260C\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'th0-$in{res}' using 1:2:3:4 with yerrorbars title \"min/ave/max temp\"\n";
};
#
# UV
if ($in{'type'} eq "uv")
{
print GP "set title \"Line Chart for UV index from $in{'start'} to $in{'end'}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%.1f \"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'uv0-$in{res}' using 1:2 with lines title \"uv index\"\n";
};
#
# UV MIN/MAX
if ($in{'type'} eq "uv-minmax")
{
print GP "set title \"Min/Max Chart for UV index from $in{'start'} to $in{'end'}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%.1f \"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'uv0-$in{res}' using 1:2:3:4 with yerrorbars title \"min/ave/max uv index\"\n";
};
#
# INDOOR COOLING
if ($in{'type'} eq "cooling")
{
print GP "set title \"Line Chart for Cooling Devices Temperature from $in{'start'} to $in{'end'}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%.1f \260C\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 't0-$in{res}' using 1:2 with lines title \"wine cooling\", 't1-$in{res}' using 1:2 with lines title \"freezer\", 'th4-$in{res}' using 1:2 with lines title \"fridge\"\n";
};
#
# INDOOR TEMP
if ($in{'type'} eq "indoor")
{
print GP "set title \"Line Chart for Indoor Temperature from $in{'start'} to $in{'end'}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%.1f \260C\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'thb0-$in{res}' using 1:2 with lines title \"EG\", 'th1-$in{res}' using 1:2 with lines title \"OG\", 'th2-$in{res}' using 1:2 with lines title \"server room\", 'th3-$in{res}' using 1:2 with lines title \"attic\"\n";
};
#
# BARO
if ($in{'type'} eq "baro")
{
print GP "set title \"Line Chart for Barometic Pressure from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%g mb\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'thb0-$in{res}' using 1:11 with lines title \"pressure\"\n";
}
#
# BARO MIN MAX
if ($in{'type'} eq "baro-minmax")
{
print GP "set title \"Min/Max Chart for Barometic Pressure from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%g mb\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'thb0-$in{res}' using 1:11:12:13 with yerrorbars title \"min/ave/max pressure\"\n";
}
#
# SEALEVEL BARO
if ($in{'type'} eq "seabaro")
{
print GP "set title \"Line Chart for Sea-Level Barometic Pressure from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%g mb\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'thb0-$in{res}' using 1:14 with lines title \"pressure\"\n";
}
#
# SEALEVEL BARO MIN MAX
if ($in{'type'} eq "seabaro-minmax")
{
print GP "set title \"Min/Max Chart for Sea-Level Barometic Pressure from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%g mb\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'thb0-$in{res}' using 1:14:15:16 with yerrorbars title \"min/ave/max pressure\"\n";
}
#
# HUM
if ($in{'type'} eq "hum")
{
print GP "set title \"Line Chart for Humidity from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%g %%\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'th0-$in{res}' using 1:8 with lines title \"humidity\"\n";
}
#
# HUM MIN MAX
if ($in{'type'} eq "hum-minmax")
{
print GP "set title \"Min/Max Chart for Humidity from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%g %%\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'th0-$in{res}' using 1:8:9:10 with yerrorbars title \"min/ave/max humidity\"\n";
}
#
# RAIN
if ($in{'type'} eq "rain")
{
print GP "set my2tics 2\n";
print GP "set grid y2tics nomy2tics\n";
print GP "set title \"Min/max Chart for Rain Rate and Bar Chart for Rain Fall from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%g mm/h\"\n";
print GP "set format y2 \"%g mm\"\n";
print GP "set ytics border nomirror norotate autofreq\n"; 
print GP "set y2tics border nomirror norotate autofreq\n"; 
print GP "set yrange [0:]\n";
print GP "set y2range [0:]\n";
print GP "set boxwidth 0.7 relative\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'rain0-$in{res}' using 1:5 axis x1y2 with boxes fs solid 0.5 title \"rain fall [mm]\", 'rain0-$in{res}' using 1:2:3:4 axis x1y1 with yerrorbars title \"min/ave/max rain rate [mm/h]\"\n";
}
#
# WIND
if ($in{'type'} eq "wind")
{
print GP "set title \"Wind and Gust Speed from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%g m/s\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'wind0-$in{res}' using 1:4 with lines title \"gust speed\",'wind0-$in{res}' using 1:6 with lines title \"average wind speed\"\n";
}
#
# WIND MIN MAX
if ($in{'type'} eq "wind-minmax")
{
print GP "set title \"Min/Max Chart for Gust and average Wind Speed from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set format y \"%g m/s\"\n";
print GP "set xrange [ \"$in{'start'}\" : \"$in{'end'}\" ] noreverse nowriteback\n";
print GP "plot 'wind0-$in{res}' using 1:5:3:4 with yerrorbars title \"min/ave/max gust and wind speed\"\n";
}
#
# WIND SDIR
if ($in{'type'} eq "wind-sdir")
{
print GP "set size square\n";
print GP "set polar\n";
print GP "set xdata\n";
print GP "set angles degrees\n";
print GP "set grid polar\n";
print GP "set mxtics 2\n";
print GP "set mytics 2\n";
print GP "set grid noxtics nomxtics ytics mytics noztics nomztics nox2tics nomx2tics noy2tics nomy2tics nocbtics nomcbtics\n";
print GP "set title \"Max Wind Speed per Direction ($in{start})\" 0.000000,0.000000  font \"\"\n";
print GP "set terminal png $myfont nocrop size 300,300\n";
print GP "set format x \"%g m/s\"\n";
print GP "set format y \"%g m/s\"\n";
print GP "set xrange [-30:30]\n";
print GP "set yrange [-30:30]\n";
print GP "set label \" N \" at first -1, first 28\n";
print GP "set label \" S \" at first -1, first -26\n";
print GP "set label \" E \" at first 27, first 1\n";
print GP "set label \" W \" at first -30, first 1\n";
print GP "plot '< /usr/local/bin/ser.pl $path/sdir0-$in{res} $in{start}' using (90-\$1*30):2 with filledcurves title \"gust speed\"\n";
}
#
# WIND TDIR
if ($in{'type'} eq "wind-tdir")
{
print GP "set size square\n";
print GP "set polar\n";
print GP "set xdata\n";
print GP "set angles degrees\n";
print GP "set grid polar\n";
print GP "set mxtics 5\n";
print GP "set mytics 5\n";
print GP "set grid noxtics nomxtics ytics mytics noztics nomztics nox2tics nomx2tics noy2tics nomy2tics nocbtics nomcbtics\n";
print GP "set title \"Wind Directions over time ($in{start})\" 0.000000,0.000000  font \"\"\n";
print GP "set terminal png $myfont nocrop size 300,300\n";
print GP "set format x \"%g %%\"\n";
print GP "set format y \"%g %%\"\n";
print GP "set xrange [-100:100]\n";
print GP "set yrange [-100:100]\n";
print GP "set label \" N \" at first -5, first 95\n";
print GP "set label \" S \" at first -5, first -92\n";
print GP "set label \" E \" at first 90, first 2\n";
print GP "set label \" W \" at first -100, first 2\n";
print GP "plot '< /usr/local/bin/ser.pl $path/tdir0-$in{res} $in{start}' using (90-\$1*30):2 with filledcurves title \"duration\"\n";
}
#
#
# TEMP 3D
if ($in{'type'} eq "temp-3d")
{
print GP "set title \"Daily Curves for Temperature from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set ytics 3\n";
print GP "set nomytics\n";
print GP "set terminal png $myfont nocrop size 610,400\n";
print GP "set format x \"%d.\\n%b\"\n";
print GP "set format y \"%g h\"\n";
print GP "set format z \"%g °C\"\n";
print GP "set yrange [0:24]\n";
print GP "set pm3d\n";
print GP "splot '< /usr/local/bin/day.pl $path/th0-$in{res} $in{start} $in{end}' using 2:4:6 with pm3d title \"\"\n";
}
#
#
# HUM 3D
if ($in{'type'} eq "hum-3d")
{
print GP "set title \"Daily Curves for Humidity from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set ytics 3\n";
print GP "set nomytics\n";
print GP "set terminal png $myfont nocrop size 610,400\n";
print GP "set format x \"%d.\\n%b\"\n";
print GP "set format y \"%g h\"\n";
print GP "set format z \"%g %%\"\n";
print GP "set yrange [0:24]\n";
print GP "set pm3d\n";
print GP "splot '< /usr/local/bin/day.pl $path/th0-$in{res} $in{start} $in{end}' using 2:4:12 with pm3d title \"\"\n";
}
#
#
# BARO 3D
if ($in{'type'} eq "baro-3d")
{
print GP "set title \"Daily Curves for Barometic Pressure from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set ytics 3\n";
print GP "set nomytics\n";
print GP "set terminal png $myfont nocrop size 610,400\n";
print GP "set format x \"%d.\\n%b\"\n";
print GP "set format y \"%g h\"\n";
print GP "set format z \"%g mb\"\n";
print GP "set yrange [0:24]\n";
print GP "set pm3d\n";
print GP "splot '< /usr/local/bin/day.pl $path/thb0-$in{res} $in{start} $in{end}' using 2:4:15 with pm3d title \"\"\n";
}
#
#
# SEALEVEL BARO 3D
if ($in{'type'} eq "seabaro-3d")
{
print GP "set title \"Daily Curves for Sea-Level Barometic Pressure from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set ytics 3\n";
print GP "set nomytics\n";
print GP "set terminal png $myfont nocrop size 610,400\n";
print GP "set format x \"%d.\\n%b\"\n";
print GP "set format y \"%g h\"\n";
print GP "set format z \"%g mb\"\n";
print GP "set yrange [0:24]\n";
print GP "set pm3d\n";
print GP "splot '< /usr/local/bin/day.pl $path/thb0-$in{res} $in{start} $in{end}' using 2:4:18 with pm3d title \"\"\n";
}
#
#
# WIND 3D
if ($in{'type'} eq "wind-3d")
{
print GP "set title \"Daily Curves for Wind Speed from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set ytics 3\n";
print GP "set nomytics\n";
print GP "set terminal png $myfont nocrop size 610,400\n";
print GP "set format x \"%d.\\n%b\"\n";
print GP "set format y \"%g h\"\n";
print GP "set format z \"%g m/s\"\n";
print GP "set yrange [0:24]\n";
print GP "set pm3d\n";
print GP "splot '< /usr/local/bin/day.pl $path/wind0-$in{res} $in{start} $in{end}' using 2:4:9 with pm3d title \"\"\n";
}
#
#
# RAIN 3D
if ($in{'type'} eq "rain-3d")
{
print GP "set title \"Daily Curves for Rain Fall per hour from $in{start} to $in{end}\" 0.000000,0.000000  font \"\"\n";
print GP "set ytics 3\n";
print GP "set nomytics\n";
print GP "set terminal png $myfont nocrop size 610,400\n";
print GP "set format x \"%d.\\n%b\"\n";
print GP "set format y \"%g h\"\n";
print GP "set format z \"%g mm\"\n";
print GP "set yrange [0:24]\n";
print GP "set pm3d\n";
print GP "splot '< /usr/local/bin/day.pl $path/rain0-$in{res} $in{start} $in{end}' using 2:4:9 with pm3d title \"\"\n";
}
#
#
close GP
