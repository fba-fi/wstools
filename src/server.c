#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "proto.h"

#define BACKLOG 10
#define BUFFERSIZE 100
#define MAXFD 15

float wind=0;
int dir=180;
float temp=0;

int new_connection(int fd, fd_set *master_fds)
{
	int newfd, addrlen;
	struct sockaddr_in remoteaddr;

	addrlen = sizeof(remoteaddr);
	if ((newfd=accept(fd, (struct sockaddr*)&remoteaddr, &addrlen)) == -1) {
		perror("Server: accept");
		return -1;
	}

	/* success, so add to master fd */
	FD_SET(newfd, master_fds);
	printf("Server(%d): New connection\n", newfd);
	return newfd;
}

int generate_data(char *buf, int bufsize)
{
	double wind_min, wind_avg, wind_max;
	int dir_min, dir_avg, dir_max;
	double water_temp, sensor_temp, battery_volt;
	float rf_level;
	char error1, error2;
	char str[BUFFERSIZE];
	int bytes;
	
	wind += (rand() % 100) / 50.0 - 1;
	wind_min = wind - (rand() % 300) / 50.0;
	wind_max = wind + (rand() % 300) / 50.0; 
	wind_avg = (wind + wind_min + wind_max) / 3.0;
	wind = wind < 0 ? 0 : wind; 
	wind_min = wind_min < 0 ? 0 : wind_min;
	wind_max = wind_max < 0 ? 0 : wind_max;
	wind_avg = wind_avg < 0 ? 0 : wind_avg;

	dir += (rand() % 11) - 5; 
	dir = abs(dir) % 360;
	dir_min = (dir - rand()%10) % 360;
	dir_max = (dir + rand()%10) % 360;
	dir_avg = dir;

	temp += (rand() % 100) / 50.0 - 1;
	water_temp = temp;
	sensor_temp = (rand() % 100) / 2.0;

	battery_volt = 6 + (rand() % 100) / 50.0 - 2;
	rf_level = (rand() % 100) / 50.0;
	error1 = '0' + rand() % 2;
	error2 = '0' + rand() % 2;

	bzero(buf, bufsize);
	bytes=sizeof(str);
	snprintf(str, bytes, "%.1f;%.1f;%.1f;", wind_min, wind_avg, wind_max);
	strncat(buf, str, bufsize);
	snprintf(str, bytes, "%d;%d;%d;", dir_min, dir_avg, dir_max);
	strncat(buf, str, bufsize);
	snprintf(str, bytes, "%.1f;%.1f;", water_temp, sensor_temp);
	strncat(buf, str, bufsize);
	snprintf(str, bytes, "%.1f;%.1f;", battery_volt, rf_level);
	strncat(buf, str, bufsize);
	snprintf(str, sizeof(buf), "%c;%c", error1, error2);
	strncat(buf, str, bufsize);

	return 0;
}

int handle_connection(int fd, fd_set *master_fds)
{
	int bytes;
	char buf[BUFFERSIZE];

	/* just to be sure */
	bzero(buf, sizeof(buf));

	/* wait for client to speak, leave -1 for NULL char */
	if ((bytes = recv(fd, buf, sizeof(buf) - 1, 0)) > 0 ) {
		printf("Server(%d): received: %s\n", fd, buf);
	} else if (bytes == 0) {
		printf("Server(%d): Client disconnected\n", fd);
	} else if (bytes < 0) {
		perror("Server: recv");
	}

	/* make up response */
	generate_data(buf, sizeof(buf));

	/* talk back to client */
	if ((bytes = send(fd, buf, strlen(buf), 0)) == -1) {
		perror("Server: send");
	}

	/* end connection */
	FD_CLR(fd, master_fds);
	close(fd);

	printf("Server(%d): Closed connection\n", fd);
	return bytes;
}


int main(int argc, char *argv[])
{
	int listen_fd, fd;
	int yes = 1;
	int addrlen;
	struct sockaddr_in addr;
	fd_set master_fds, read_fds;

	printf("Server: init\n");

	/* get new socket & allow immediately reusing reserved port */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	/* server port and address */
	bzero(&addr, sizeof(addr));	
	addr.sin_family = AF_INET;	/* host byte order */
	addr.sin_port = htons(SERVER_PORT);	/* short, network byte order */
	addr.sin_addr.s_addr = INADDR_ANY;	/* host ip */

	if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr))
								 == -1 ) {
		perror("Server: bind");
		exit(1);
	}

	if (listen(listen_fd, BACKLOG) == -1 ) {
		perror("Server: listen");
		exit(1);
	}

	/* keep record of connections */
	FD_ZERO(&master_fds);
	FD_SET(listen_fd, &master_fds);

	/* main loop */
	while (1) {

		/* select modifies read_fds */
		read_fds = master_fds;
		if (select(MAXFD+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("Server: select");
			continue;
		}

		/* iterate read_fds for new data/connection */
		for (fd = 0; fd <= MAXFD; fd++) {
			if (FD_ISSET(fd, &read_fds)) {
				if (fd == listen_fd) {
					/* create and store to master */
					new_connection(fd, &master_fds);
				} else {
					/* handle and delete from master */ 
					handle_connection(fd, &master_fds);
				}
			}
		}
	}

	/* should never get here */
	close(listen_fd);
}
