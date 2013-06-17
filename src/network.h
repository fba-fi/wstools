/*

	network.h

	Copyright (C) 2007 Mikko Vatanen

	tcp/ip protocol for weather station

*/

#ifndef WS_NETWORK_H
#define WS_NETWORK_H 1

extern int connect_ip(char *ip, int port);
extern int send_data(int sockfd, char *buf); /* BUGBUG: msg not NULL terminated? */
extern int get_data(int sockfd, char *buf, int bufsize, char *msg); /* BUGBUG: msg not NULL terminated? */

#endif
