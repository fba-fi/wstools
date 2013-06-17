/*
	datautils.h

	datatypes & handling routines
*/

#include "proto.h"

#ifndef DATAUTILS_H 
#define DATAUTILS_H  1

/* internal data format */
struct weatherdata {
        time_t time;            /* no time from receiver */
        double wind_min;
        double wind_avg;
        double wind_max;
        int dir_min;
        int dir_avg;
        // 31.2.2008 Added vector components for proper averaging
        double dir_avg_x;
        double dir_avg_y;

        int dir_max;
        double water_temp;
        double sensor_temp;
        double battery_voltage;
        double rf_level;
        char error1;
        char error2;    
};

extern void weatherdata_copy(struct weatherdata *dest, struct weatherdata *src);
extern void weatherdata_clear(struct weatherdata *data);

extern void weatherdata_to_string(struct weatherdata *data, char *buf, size_t n);
extern void weatherdata_to_tabstring(struct weatherdata *data, char *buf, size_t n);
extern void weatherdata_to_html(struct weatherdata *data, char *buf, size_t n);

extern int string_to_weatherdata(char *buf, size_t n, struct weatherdata *data);

extern void weatherdata_statistic_add(struct weatherdata *dest, struct weatherdata *src);
extern void weatherdata_statistic_calc(struct weatherdata *dest, int i);

#endif
