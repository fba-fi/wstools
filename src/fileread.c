#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{

	FILE *f;
	char fname[100] = "data/2007-08-09.dat";
	char lines[15][100];
	struct stat buf;
	char str[200];

	int i, hour;

	time_t starttime;
	struct tm *loctime;

	stat(fname, &buf);

	if ((f = fopen(fname, "r")) == NULL) {
		printf("error1\n");
		return 1;
	}
	
	/* Hour of day */
	starttime = time(NULL);
	loctime = localtime(&starttime);

	printf("time: %i\n", loctime->tm_hour);

	if (fseek(f, buf.st_size - loctime->tm_hour * 300 * 70, SEEK_SET)!=0) {
		printf("error\n");
		return 1;
	}

	fgets(str, sizeof(str), f);

	while ( fgets(str, sizeof(str), f) != NULL ) {
		printf("%s", str);
	}

	printf("done\n");
	
	return 0;

}
