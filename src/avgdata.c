/* 
 *	avgdata.c 
 *
 * 	Input data file and calculate averages.  
 * 	Output in text or html
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "proto.h"
#include "datautils.h"
#include "common.h"
#include "options.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>


struct avgdata_options options;

/*
 *	handle_data(char *buf, int bufsize, struct weatherdata avgdata
 *
 *	calculate and print average values for given data
 *
 *	return -1 when time in data > options.endtime
 *
 */

int
average_and_print(char *buf, int bufsize, time_t *data_time)
{
	static struct weatherdata data, average_data;
	static char tmpstr[BUFSIZE];
	static int row_count = 0;

	if ( string_to_weatherdata(buf, sizeof(buf), &data) != 0 ) return 0;

	*data_time = data.time;

	/* skip until starttime, stop at endtime */
	if ( data.time <= options.starttime ) return 0;
	if ( data.time > options.endtime ) return -1;

	if (row_count == 0) weatherdata_copy(&average_data, &data);

	/* calculate statistics and print line */
	if ( (options.avgtime_min == 0) || ((data.time % options.avgtime_min) == 0) ) {

		/* if avgtime = 0, just print */
		if ( options.avgtime_min != 0) weatherdata_statistic_calc(&average_data, row_count);

		average_data.time = data.time;

		/* Print html or tab delimited */
		if ( options.format_html ) {
			weatherdata_to_html(&average_data, tmpstr, sizeof(tmpstr));
		} else {
			weatherdata_to_tabstring(&average_data, tmpstr, sizeof(tmpstr));
		}

		printf("%s\n", tmpstr); 

		row_count = 0;

		weatherdata_copy(&average_data, &data);

	} else {

		weatherdata_statistic_add(&average_data, &data);	
		row_count++;

	}

	return 0;
}

/* make filename for given time */
void
time_to_filename(char *buf, int bufsize, time_t t) 
{
	static struct tm *loctime;
	static char timestr[BUFSIZE];
	
	loctime = localtime(&t);
	strftime(timestr, sizeof(timestr), "%F", loctime);
	snprintf(buf, bufsize, "%s/%s.%s", options.datadir, timestr, FILEEXT);
}

int
main(int argc, char *argv[])
{
	FILE *srcfile = NULL;

	char filename[BUFSIZE];
	char buf[BUFSIZE];

	static time_t data_time; 

	/* check command line options */
	avgdata_parse_options(argc, argv, &options);

	/* print html table header */
	if ( options.format_html ) {
		printf("<table>\n");
		printf("<tr><th>Time</th>");
		printf("<th>dMin</th><th>dAvg</th><th>dMax</th>");
		printf("<th>sMin</th><th>sAvg</th><th>sMax</th>");
		printf("<th>Temp</th><th>Water</th></tr>\n");
	}

	bzero(filename, sizeof(filename));
	bzero(buf, sizeof(buf));

	data_time = options.starttime;

	while ( data_time <= options.endtime ) {

		/* generate new filename. if same as last file; then skip to next day */
		time_to_filename(buf, sizeof(buf), data_time);
		if (strstr(filename, buf) != NULL) {
			/* set hours to 00:00:00, add 24 hours */
			data_time = data_time - ( data_time % 24 * 60 * 60 ) + 24 * 60 * 60;
			continue;
		}
		snprintf(filename, sizeof(filename), "%s", buf);

		/* open file */
		if ( (srcfile = fopen(filename, "r")) == NULL ) {
			perror(filename);
			continue;
		}

		/* process file */
		while ( fgets(buf, sizeof(buf), srcfile) != NULL ) {

			average_and_print(buf, sizeof(buf), &data_time);
		}

		/* close file */
		fclose(srcfile);
		srcfile = NULL;
	}

	/* print html table footer */
	if ( options.format_html ) {
		printf("</table>\n");
	}

	return 0;

}
