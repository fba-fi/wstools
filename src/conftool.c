/*

	conftool.c

	Copyright (C) 2007 Mikko Vatanen

	Software for reading and setting parameters for
	Kaale's weather station

*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

/* fstat() */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "proto.h"
#include "common.h"
#include "network.h"


#define DEBUG

void sig_alrm(int sig)
{
	switch ( sig )
	{
		case SIGALRM: break;
	}
}


int main(int argc, char *argv[])
{
	int sockfd;
	struct sigaction act;
	char fname[BUFSIZE], timestr[BUFSIZE], buf[BUFSIZE];
	char stringbuffer[50][BUFSIZE];
	int i;
	time_t now;

	char *str, *ptr, *pEnd;
	double rf_level;

	struct tm *loctime;

	struct stat statbuf;

	FILE *f;

	/* cgi variables */
	int postsize;

	/* print html headers */ 
	printf("Content-Type: text/html;charset=iso-8859-1\n\n");
	printf("<html><head>\n");
	printf("<style type=\"text/css\">");
	printf(" p { margin: 0; padding: 0;}");
	printf(" h1 { font-size: 120%; padding: 0; margin-bottom: 0; } ");
	printf(" form { float:left } ");
	printf(" hr { clear: both } ");
	printf(" </style>");
	/* printf("<meta http-equiv=\"refresh\" content=\"5\">"); */
	printf("</head><body>\n");

	/* get post data */
	str = getenv("CONTENT_LENGTH");
	if (str!=NULL) postsize = atoi(str);
	else postsize=0;
	fgets(buf, postsize+1, stdin);
	ptr=buf;
	while (((*ptr)!='=')&&((*ptr)!='\0')) {
		ptr++;
	}
	if (*ptr!='\0') ptr++;

	/* setup variables */

	/* setup signal handler */
	act.sa_handler = sig_alrm;
	act.sa_flags = 0;
	sigaction(SIGALRM, &act, 0);
	
	/* check it's valid float and greater than 0 */
	errno = 0;
	rf_level = strtod(ptr, &pEnd);

	/* parameter was given, so send it to weather station */
	//printf("%s", ptr);


	if (strstr(ptr, "reset") != NULL) {

		/* Parameter was reset command */

		/* connect */
		if ((sockfd = connect_ip(SERVER_IP, SERVER_PORT)) == -1) {
			return -1;
		}

		/* generate parameter string */
		snprintf(buf, sizeof(buf), "%s", MSG_RESET, ptr);

		/* send parameters to server */
		if (send_data(sockfd, buf, sizeof(buf)) != 0) {
			return -1;
		}

		close(sockfd);

		sleep(5);

		printf("Reset successful!");

	} else if (rf_level > 0) {

		/* Parameter was RF trigger level */

		/* connect */
		if ((sockfd = connect_ip(SERVER_IP, SERVER_PORT)) == -1) {
			return -1;
		}

		/* generate parameter string */
		snprintf(buf, sizeof(buf), "%s;%s\r\n", MSG_SET_PARAMS, ptr);

		/* send parameters to server */
		if (send_data(sockfd, buf, sizeof(buf)) != 0) {
			return -1;
		}

		close(sockfd);

		sleep(1);

		printf("Level set succesfully!");
	}

	/* fail connection on timeout */
	alarm(FAILTIME);

	if ((sockfd = connect_ip(SERVER_IP, SERVER_PORT)) == -1) {
		return 1;
	}

	/* start talking to server */
	if (get_data(sockfd, buf, sizeof(buf), MSG_GET_PARAMS) <= 0) {
		return 1;
	}

	close(sockfd);

	rf_level = strtod(buf, &pEnd);

	/* clear interrupt */
	alarm(0);

	/* Page title */
	printf("<h1>Weather Station Configuration Tool</h1>");
	printf("<p>Version 1.0, Copyright (C) 2007 Mikko Vatanen\n</p><hr>");
	
	/* Parameter form */
	printf("<form method=\"post\" action=\"/cgi-bin/conftool\">\n");
	printf("<label>RF Trigger Level: </label>");
	printf("<input type=\"text\" name=\"level\" value=\"%2.2f\">\n", rf_level);
	printf("<input type=\"submit\" value=\"Set level\">\n");
	printf("</form>\n");

	/* Reset form */
	printf("<form method=\"post\" action=\"/cgi-bin/conftool\">\n");
	printf("<input type=\"hidden\" name=\"level\" value=\"reset\">\n");
	printf("&nbsp;&nbsp;&nbsp;&nbsp;<input type=\"submit\" value=\"Reset WStation\">\n");
	printf("</form>");

	/* Refresh data */
	printf("<form method=\"post\" action=\"/cgi-bin/conftool\">\n");
	printf("<input type=\"hidden\" name=\"level\" value=\"\">\n");
	printf("&nbsp;&nbsp;&nbsp;&nbsp;<input type=\"submit\" value=\"Refresh page\">\n");
	printf("</form>");

	printf("<br><hr>");

	/* tail newest data file */

	now = time(NULL);
	loctime = localtime(&now);

	strftime(timestr, sizeof(timestr), "%F", loctime);
	sprintf(fname, "/home/wwwadmin/htdocs/saa/%s/%s.%s", DATADIR, timestr, FILEEXT);
	printf("<b>%s</b><br><pre>", fname);

	stat(fname, &statbuf);

	if ((f = fopen(fname, "r")) != NULL) {

		i = 0;

		if (fseek(f, statbuf.st_size - 40 * 65, SEEK_SET ) == 0) {

			/* ignore partial line first */
			fgets(buf, sizeof(buf), f);

			/* store data for reverse order */
			while (fgets(buf, sizeof(buf), f) != NULL && i < 40) {
				strncpy(stringbuffer[i++], buf, BUFSIZE);
			}	
		}

		fclose(f);

	}

	/* Print data in reverse order */
	while ( i >= 0 ) {
		printf("%s", stringbuffer[i--]);
	}

	printf("</pre>\n");

	/* print html footer */
	printf("</body></html>\n");

	return 0;
}


