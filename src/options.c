#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#define _X_OPEN_SOURCE
#define _GNU_SOURCE

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

#include <regex.h>
#include <unistd.h>
#include <time.h>

#include "common.h"
#include "error.h"
#include "options.h"
#include "string.h"
#include "proto.h"

static int debug_flag = 1;

void
wsclient_print_help()
{
	static char msg[] = 
	"Usage: wsclient --ip SERVER IP --port SERVER PORT --dir DATADIR \n";

	printf("%s", msg);
}

void
strtime(time_t *t, char *buf) {
	static struct tm tm;
	
	strptime(buf, FMT_TIME, &tm);
	*t = mktime(&tm);
}

int
wsclient_parse_options(int argc, char **argv, struct wsclient_options opt)
{
	int c;
	int option_index;
	int success = 0;
	regex_t regex;
	regmatch_t regmatch;
	

	bzero(&opt, sizeof(struct wsclient_options));

	while (1) {

		static struct option ws_options[] = {
			{"help", no_argument, 0, 'h'},
			{"version", no_argument, 0, 'v'},
			{"ip", required_argument, 0, 'i'},
			{"port", required_argument, 0, 'p'},
			{"dir", required_argument, 0, 'd'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "hvs:p:d:", ws_options, &option_index);

		// if (optarg) printf("%s\n", optarg);

		if ( c == -1 )
			break;

		switch (c) {
			case 'h':
				printf("help message\n");
				break;

			case 'v':
				printf("version message\n");
				break;

			case 'i':
				snprintf(opt.server_ip, sizeof(opt.server_ip), "%s",  optarg);
				break;

			case 'p':
				opt.server_port = atoi(optarg);
				break;

			case 'd':
				snprintf(opt.datadir, sizeof(opt.datadir), "%s", optarg);
				break;

			case 'D':
				debug_flag = 1;
				break;

			case '?':
				break;

			default:
				break;
		}
	}

	/* IP must be 0.0.0.0 - 255.255.255.255 */
	/* BUGBUG: validate ranges? */
	regcomp(&regex, "^[0-9]\\{1,3\\}\\.[0-9]\\{1,3\\}\\.[0-9]\\{1,3\\}", 0);
	if (regexec(&regex, opt.server_ip, 1, &regmatch, 0) != 0) {
		wsclient_print_help();
		exit(EXIT_FAILURE);
	}

	/* data dir must exist and be writable */
	if ( access(opt.datadir, W_OK) != 0 ) {
		perror("Cannot write to datadir!");
		exit(EXIT_FAILURE);
	} 

	return success;
}


int
avgdata_parse_options(int argc, char **argv, struct avgdata_options *opt)
{
	int c;
	int option_index;
	int success = 0;
	char *pEnd;

	/* set program defaults first */
	bzero(opt, sizeof(opt));

	snprintf(opt->datadir, sizeof(opt->datadir), "%s", ".");
	opt->range_hours = 12;
	opt->avgtime_min = 15 * 60; 
	opt->endtime = time(NULL);
	opt->starttime = opt->endtime - ( opt->range_hours * 60 * 60 );

	while (1) {

		static struct option ws_options[] = {
			{"help", no_argument, 0, 'h'},
			{"version", no_argument, 0, 'v'},

			{"starttime", required_argument, 0, 's'},
			{"endtime", required_argument, 0, 'e'},
			{"avgtime", required_argument, 0, 'a'},
			{"range", required_argument, 0, 'r'},

			{"datadir", required_argument, 0, 'd'},
			{"filename", required_argument, 0, 'f'},

			{"html", no_argument, 0, 'H'},
			{"text", no_argument, 0, 't'},

			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "hvs:e:a:r:d:f:Ht", ws_options, &option_index);

		if ( c == -1 )
			break;

		switch (c) {
			case 'h':
				printf("help message\n");
				break;

			case 'v':
				printf("version message\n");
				break;

			/* starttime, avgtime, avgperiod, range_hours */
			case 's':
				strtime(&opt->starttime, optarg);
				break;
			case 'e':
				strtime(&opt->endtime, optarg);
				break;
			case 'a':
				opt->avgtime_min = strtod(optarg, &pEnd) * 60;
				break;
			case 'r':
				opt->range_hours = strtod(optarg, &pEnd);
				opt->starttime = opt->endtime - ( opt->range_hours * 60 * 60 );
				break;

			/* datadir, filename */
			case 'd':
				snprintf(opt->datadir, sizeof(opt->datadir), "%s", optarg);
				break;

			/* text, html */
			case 't':
				opt->format_html = 0;
				break;
			case 'H':
				opt->format_html = 1;
				break;

			/* error */
			case '?':
				break;

			default:
				break;
		}
	}


	/* data dir must exist and be readable */
	if ( access(opt->datadir, R_OK) != 0 ) {
		perror(opt->datadir);
		exit(EXIT_FAILURE);
	}

	return success;
}

