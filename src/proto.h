/*
	proto.h

	defines constants for receiving protocol
*/

#include <time.h>

#ifndef _PROTO_H_
#define _PROTO_H_

#define PI 3.14159265358979323846264338327

#define SERVER_IP	"192.168.0.5"
#define SERVER_PORT	10001

/* buffer size for receive / send buffers */
#define BUFSIZE 		255 

/* default polling time, TODO: override in settings */
#define POLLTIME 5 
#define FAILTIME 3

/* send to reset receiver */
#define MSG_RESET 		"reset\r\n"

/* send to get or set receiver params */
#define MSG_GET_PARAMS 		"gpara\r\n"
#define MSG_SET_PARAMS 		"spara" 

/* string formats for receiver settings */
#define FMT_AVG_TIME 		"%d" 	/* 1-60 */
#define FMT_TRIGGER_VOLTAGE 	"%.3f"	/* 0-9.999 */

/*  send to receive data */
#define MSG_GET_DATA 		"gdata\r\n"

/* field separator character */
#define FMT_SEPARATOR ";"

/* string formats for data fields */
#define FMT_TIME 		"%F %T"	/* ISO 8691 / YYYY-MM-DD HH-MM-SS*/
#define FMT_WIND 		"%.1f"	/* 0 - 99.0 */
#define FMT_DIR 		"%d"	/* 0 - 360 */
#define FMT_WATER_TEMP 		"%.1f"	/* -99.0 - 99.0 */
#define FMT_SENSOR_TEMP 	"%.1f"	/* -99.0 - 99.0 */
#define FMT_BATTERY_VOLTAGE 	"%.1f"	/* 0 - 99.0 */
#define FMT_RF_LEVEL 		"%.1f"	/* 0 - 99.0 */


/* parameter data */
struct recparams {
	int average_time;
	double trigger_voltage;
};

#endif
