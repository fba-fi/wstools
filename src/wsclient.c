/*
	wsclient.c

	Copyright (C) 2007 Mikko Vatanen

	Client for retrieving and storing data from weather station
	engineered 2007 by Kaale Nieminen for Yyteri Surf Center.

*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include <errno.h>

#include "proto.h"
#include "common.h"
#include "network.h"
#include "options.h"

#define DEBUG

#define CONFIGFILE		"/etc/weatherd/client.conf"

void sig_alrm(int sig)
{
	switch ( sig )
	{
		case SIGALRM: break;
	}
}


/*

	Writes received data to a file	

*/

int
handle_data(char *buf, const int bufsize, const time_t polltime, char *datadir)
{
	struct tm *loctime;
	char timestr[100];
	char fname[100];
	FILE *logfile;

	/* open file named YYYY-MM-DD.txt for writing */
	loctime = localtime(&polltime);

	strftime(timestr, sizeof(timestr), "%F", loctime);
	sprintf(fname, "%s/%s.%s", datadir, timestr, FILEEXT);

	if ((logfile = fopen(fname, "a")) == NULL) {
		perror("Client: fopen()");
		return -1;
	}

	strftime(timestr, sizeof(timestr), "%F %T", loctime);

	#ifdef DEBUG
	printf("Client: %s;%s\n", timestr, buf);
	#endif

	fprintf(logfile, "%s;%s\n", timestr, buf);

	fclose(logfile);
	
	return 0;
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct sigaction act;
	char buf[BUFSIZE];
	time_t polltime;

	struct wsclient_options opt;

	/* parse command line options */
	wsclient_parse_options(argc, argv, opt);

	/* begin program */
	printf("Weather Client 1.0, copyright (C) 2007 Mikko Vatanen\n");

	/* setup signal handler */
	act.sa_handler = sig_alrm;
	act.sa_flags = 0;
	sigaction(SIGALRM, &act, 0);

	while (1) {
		/* poll at even seconds */
		sleep(abs(POLLTIME - time(NULL) % POLLTIME));	

		/* raise signal before next polling */
		alarm(FAILTIME);

		/* store polling time */
		polltime = time(NULL);

		if ((sockfd = connect_ip(SERVER_IP, SERVER_PORT)) == -1) {
			/* connect_ip prints error, just skip rest */
			continue;
		}

		/* start talking to server */
		if (get_data(sockfd, buf, sizeof(buf), MSG_GET_DATA) <= 0) {
			continue;
		}

		handle_data(buf, sizeof(buf), polltime, opt.datadir);

		close(sockfd);

		#ifdef DEBUG
		printf("Client: connection closed\n");
		#endif

		/* no interrupt before next poll */
		alarm(0);
	}
}
