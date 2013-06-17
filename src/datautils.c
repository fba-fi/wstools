/*
	datautils.c

	Copyright (C) 2007 Mikko Vatanen

	Routines for converting data form semicolon delimited
	strings to record and back
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdarg.h>

#include <regex.h>
#include <time.h>
#include <math.h>
#include <string.h> 

#include "proto.h"
#include "datautils.h"

/* private declarations */
void concat_sprintf(char *dest, const char *format, ...)
	__attribute__ ((format (printf, 2,3)));

/* weather data handling */
void
weatherdata_clear(struct weatherdata *data)
{
	bzero(data, sizeof(struct weatherdata));
}

void
weatherdata_copy(struct weatherdata *dest, struct weatherdata *src)
{
	memcpy(dest, src, sizeof(struct weatherdata));
} 

/* functions for printing */
void
concat_sprintf(char *dest, const char *format, ...)
{
	va_list ap;
	char tmpstr[BUFSIZE];

	va_start(ap, format);

	snprintf(tmpstr, sizeof(tmpstr), format, ap);
	strncat(dest, tmpstr, sizeof(dest));

	va_end(ap);
}

/* print weatherdata to string */
void
weatherdata_to_string(struct weatherdata *data, char *dest, size_t dest_size)
{
	struct tm *loctime;

	bzero(dest, dest_size);

	/* print time, only if sane value */
	if ( data->time != 0 ) {
		loctime = localtime(&data->time);
		strftime(dest, dest_size, "%F %T;", loctime);
	}

	/* wind direction */
	concat_sprintf(dest, "%d;%d;%d;", data->dir_min, data->dir_avg,
						data->dir_max);

	/* wind speed */
	concat_sprintf(dest, "%.1f;%.1f;%.1f;", data->wind_min,
							data->wind_avg,
							data->wind_max);

	/* sensor temp, voltage */
	concat_sprintf(dest, "%.1f;%.1f;", data->sensor_temp, 
							data->battery_voltage);

	/* water temp, rf level  */
	concat_sprintf(dest, "%.1f;%.1f;", data->water_temp,
							data->rf_level);
	
	/* error1, error2 
	concat_sprintf(dest, "%c;%c", data->error1, data->error2);
	*/
}

/* print weatherdata as html table (tab delimited) */
void
weatherdata_to_html(struct weatherdata *data, char *dest, size_t dest_size)
{
	struct tm *loctime;

	bzero(dest, dest_size);

	/* print valid time, else just table row header  */
	if ( data->time != 0 ) {
		loctime = localtime(&data->time);
		strftime(dest, dest_size, "<tr><td>%F %H:%M</td>", loctime);
	} else {
		snprintf(dest, dest_size, "<tr><td></td>");
	}
	
	/* wind direction */
	concat_sprintf(dest, "<td>%d</td><td>%d</td><td>%d</td>", 
					data->dir_min, data->dir_avg, data->dir_max);

	/* wind speed */
	concat_sprintf(dest, "<td>%.1f</td><td>%.1f</td><td>%.1f</td>",
							data->wind_min,
							data->wind_avg,
							data->wind_max);

	/* sensor temp */ 
	concat_sprintf(dest, "<td>%.1f</td>", data->sensor_temp);

	/* water temp */
	concat_sprintf(dest, "<td>%.1f</td>", data->water_temp);

	/* end html-table row */
	concat_sprintf(dest, "%s", "</tr>");


}

/* print weatherdata in human readable format (tab delimited) */
void
weatherdata_to_tabstring(struct weatherdata *data, char *dest, size_t dest_size)
{
	struct tm *loctime;

	bzero(dest, dest_size);

	/* print time, only if sane value */
	if ( data->time != 0 ) {
		loctime = localtime(&data->time);
		strftime(dest, dest_size, "%F %H:%M:%S\t", loctime);
	}

	// printf("%d\t%d\t%d\n", data->dir_min, data->dir_avg, data->dir_max);

	/* wind direction */
	concat_sprintf(dest, "%d\t%d\t%d\t", data->dir_min, data->dir_avg,
							data->dir_max);

	/* wind speed */
	concat_sprintf(dest, "%.1f\t%.1f\t%.1f\t", data->wind_min,
							data->wind_avg,
							data->wind_max);

	/* water temp */
	concat_sprintf(dest, "%.1f\t", data->water_temp);
}


/* functions for conversion */

/* parse data from string to weatherdata */
int
string_to_weatherdata(char *buf, size_t n, struct weatherdata *data)
{
	char delimiters[] = ";";
	char tmpstr[BUFSIZE];
	char *token, *pEnd;
	int count = 0;

	struct tm loctime;

	//regex_t regex;
	//regmatch_t regmatch;

	bzero(data, sizeof(struct weatherdata));

        strncpy(tmpstr, buf, sizeof(tmpstr));

	/* strip ending \n */
	if ((token = strstr(tmpstr, "\n")) != NULL) {
		*token = '\0';
	}

	/* Not enough data */
	count = 0;
	for (n=0;n<BUFSIZE;n++) {
		if (tmpstr[n] == delimiters[0]) {
			count++;
		}
	}
	if (count < 9) return 0;

	/* Parse data */
	token = strtok(tmpstr, delimiters);
	if (token == NULL) return -1;

	/* read time from string, on error read wind direction */
	//regcomp(&regex, "^[0-9]+-[0-9]+-[0-9]+ [0-9]+:[0-9]+:[0-9]+", 0);
	//if (regexec(&regex, tmpstr, 1, &regmatch, 0) != 0) {
		//printf("jee\n");
	//}

	strtoul(token, &pEnd, 10);

	if ( *pEnd != '\0') { 
		/* value was time */
		strptime(token, "%F %T", &loctime);
		data->time = mktime(&loctime);

		/* read next token */
		token = strtok(NULL, delimiters);
	} else {
		/* value was wind_min so just zero time */
		data->time = 0;
	}


	/*	int dir_min;	*/
	if (token == NULL) return -1;
	data->dir_min = atoi(token);

	/*	int dir_avg;	*/
	token = strtok(NULL, delimiters);
	if (token == NULL) return -1;
	data->dir_avg = atoi(token);

	/*	int dir_max;	*/
	token = strtok(NULL, delimiters);
	if (token == NULL) return -1;
	data->dir_max = atoi(token);

	/*	double wind_min;	*/
	token = strtok(NULL, delimiters);
	if (token == NULL) return -1;
	data->wind_min = strtod(token, &pEnd);

	/*	double wind_avg;	*/
	token = strtok(NULL, delimiters);
	if (token == NULL) return -1;
	data->wind_avg = strtod(token, &pEnd);

	/*	double wind_max;	*/
	token = strtok(NULL, delimiters);
	if (token == NULL) return -1;
	data->wind_max = strtod(token, &pEnd);

	/*	double sensor_temp;	*/
	token = strtok(NULL, delimiters);
	if (token == NULL) return -1;
	data->sensor_temp = strtod(token, &pEnd);

	/*	double bat_voltage;	*/
	token = strtok(NULL, delimiters);
	if (token == NULL) return -1;
	data->battery_voltage = strtod(token, &pEnd);

	/*	double water_temp;	*/
	token = strtok(NULL, delimiters);
	if (token == NULL) return -1;
	data->water_temp = strtod(token, &pEnd);

	/*	double rf_level;	*/
	token = strtok(NULL, delimiters);
	if (token == NULL) return -1;
	data->rf_level = strtod(token, &pEnd);

	return 0;

}

/* functions for statistics */

int rotation_dir(int d1, int d2)
{
	// Return 1 if second angle is to clockwise from first angle, else 0

	double x1, x2, y1, y2;
	double angle;

	// Calculate unit vector components from polar coordinates
	x1 = sin(( M_PI * d1) / 180);
	y1 = cos(( M_PI * d1) / 180);

	x2 = sin(( M_PI * d2) / 180);
	y2 = cos(( M_PI * d2) / 180);

	// Cross product
	angle = x1*y2 - x2*y1;

	// Sign indicates if clockwise
	if ( angle > 0 ) return 0;
	else return 1;
}

/*
	return_dir_minmax(dir1, dir2, rotation_dir)

	return greater wind direction according to rotation direction
*/
int
dir_max(int dir1, int dir2, int max_dir)
{
	return ( rotation_dir(dir1, dir2) == max_dir ? dir2 : dir1 );
}
	
void
weatherdata_statistic_add(struct weatherdata *dest, struct weatherdata *src)
{
	
	dest->wind_min = src->wind_min < dest->wind_min ? src->wind_min : dest->wind_min;
	dest->wind_avg += src->wind_avg;
	dest->wind_max = src->wind_max > dest->wind_max ? src->wind_max : dest->wind_max;

	// Minimum direction is to anti-clockwise
	dest->dir_min = dir_max(dest->dir_min, src->dir_min, 0);

	// Max direction is to clockwise
	dest->dir_max = dir_max(dest->dir_max, src->dir_max, 1);

	// 31.8.2008 sum vector components for average wind direction 
	dest->dir_avg += src->dir_avg;
	dest->dir_avg_x += sin(( M_PI * src->dir_avg) / 180);
	dest->dir_avg_y += cos(( M_PI * src->dir_avg) / 180);

	dest->water_temp += src->water_temp;
	dest->sensor_temp += src->sensor_temp;
	dest->battery_voltage += src->battery_voltage;
	dest->rf_level += src->rf_level;
}

void
weatherdata_statistic_calc(struct weatherdata *dest, int i)
{
	if (i==0) i=1;

	dest->wind_avg = dest->wind_avg / i;

	// 31.8.2008 Calculate average direction from vector components
	dest->dir_avg = rint( atan2(dest->dir_avg_x, dest->dir_avg_y) / M_PI * 180 );
	dest->dir_avg = dest->dir_avg < 0 ? dest->dir_avg + 360 : dest->dir_avg;

	dest->water_temp = dest->water_temp / i;
	dest->sensor_temp = dest->sensor_temp / i;
	dest->battery_voltage = dest->battery_voltage / i;
	dest->rf_level = dest->rf_level / i;

}

