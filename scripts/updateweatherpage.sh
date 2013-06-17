#!/bin/bash

# updateweatherdata
# Version 1.0
# Copyright (C) 2007 Mikko Vatanen
#
# Cron script for updating weather server's WWW pages
#

# START CONFIGURATION

# Generated files are stored here
WORKDIR=/home/wwwadmin/htdocs/saa

# Original data from weather station
DATADIR=/home/wwwadmin/htdocs/saa/data

# Location of executable files
EXECDIR=/home/wwwadmin/bin

# Location and filename for
TMPFILE=/tmp/weatherdata.tmp

# END CONFIGURATION

cd $WORKDIR

echo "`date +'%F %T'` Generating pages...`"

# Start weather station client again if dropped dead
CLIENTPID="`pgrep wsclient`"
if [ -z "$CLIENTPID" ] ; then
        ${EXECDIR}wsclient
fi

# Calculate graph time-range for Gnuplot
function calcxrange() {
	seconds=`date +'%s'`
	endsec=$((seconds%(30*60)))
	startsec=$((endsec+(($1-1)*60*60+45*60)))

	start=`date -d "now - $startsec second" +"%F %T" `
	end=`date -d "now - $endsec second" +"%F %T" `
	echo "set xrange [ \"$start\":]"
}

# Generate pages
time (\
	# Generate plots
	${EXECDIR}/avgdata --datadir ${DATADIR} --range 24 --avgtime 0 > ${TMPFILE}; \ 
	calcxrange 24 > /tmp/xrange.plot; \

	# Generate statistics HTML
	${EXECDIR}/avgdata --datadir ${DATADIR} --range 6 --avgtime 15 --html > ${TMPFILE}; \
	sed "/#data#/r ${EXECDIR}/index.html" > ${WORKDIR}/index.html ; \

	/usr/bin/gnuplot ${EXECDIR}/hourgraph.plot; \

	rm "$TMPFILE"
)

echo "--------------------"
