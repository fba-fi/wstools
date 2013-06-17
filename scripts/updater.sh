#!/bin/bash

while [ 1 ] ; do
	echo Generating pages...
	date +'%F %T'
	time (\
	./parser 6 html 15 > tmp.data; \
	./createhtml.sh tmp.data; \
	./parser 24 text 0 > tmp.data; \
   	./calcxrange.sh 24 > xrange.plot; \
	/usr/bin/gnuplot hourgraph.plot; \
	#scp html/* surf:/home/www/htdocs/saa/; \
	#scp data/2007-08-09.dat surf:/home/www/htdocs/saa/data/; \
	cp html/* /var/www/saa/
	scp data/2007-08-10.dat /var/www/saa/data/
 	);
	sleep 5 
done
