#ifndef WS_OPTIONS_H
#define WS_OPTIONS_H 1

#define FILEEXT "txt"

struct wsclient_options {
	int help_flag;

	char server_ip[255];
	int server_port;

	char datadir[255];
};


struct avgdata_options {

	int help_flag;

	char filename[255];
	char datadir[255];

	time_t starttime;
	time_t endtime;

	int range_hours;
	int avgtime_min;

	int format_html;
};

int wsclient_parse_options (int argc, char **argv, struct wsclient_options opt);
int avgdata_parse_options (int argc, char **argv, struct avgdata_options *opt);

#endif /* !WS_OPTIONS_H */
