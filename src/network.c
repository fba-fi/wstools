/*
	network.c

	Copyright (C) 2007 Mikko Vatanen

	Routines for tcp/ip protocol 

*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

#include "proto.h"
#include "common.h"


/*
	sets weather station parameters

	return 0 on success
	-1 on error
*/
int
send_data(int sockfd, char *buf) /* BUGBUG: msg not NULL terminated? */
{

	/* send parameters to server */
	if (send(sockfd, buf, strlen(buf), 0)  == -1) {
		perror("send");
		return -1;
	}

	return 0;
}	

/*
	request and read data from given socket
	return data in buf
	return number of read bytes
	or	0 server closed connection
		-1 other error

*/

int
get_data(int sockfd, char *buf, int bufsize, char *msg) /* BUGBUG: msg not NULL terminated? */
{
	int bytes;
	char *ptr;

	bzero(buf, bufsize);
	snprintf(buf, bufsize, "%s", msg);

	/* request for data */
	if (send(sockfd, buf, strlen(buf), 0)  == -1) {
		perror("network.c/send");
		return -1;
	}

	/* don't return any crap */
	bzero(buf, bufsize);

	/* wait for server response, leave -1 for NULL char  */
	if ((bytes = recv(sockfd, buf, bufsize - 1, 0)) > 0 ) {

		/* Strip ending CR + LF */
		if ( (ptr=strstr(buf, "\r\n")) ) {
			bzero(ptr, 2);
			bytes++;bytes++;
		}

		#ifdef DEBUG
		printf("Client: received: %s\n", buf);
		
		#endif
	} else if (bytes == 0) {
		printf("Client: Server closed the connection\n");
  	} else if (bytes < 0) {
		/* check for SIGALRM */
		if ( errno == EINTR ) {
			printf("Client: timed out\n");
		} else {
			perror("Client: recv");
		}
	}

	/* just to be sure */
	if ( bytes <= 0 ) {
		bzero(buf, bufsize);
	}

	return bytes;
}	


/*
	Open client connection to specified IP address and port
	return socketfd or -1 on error
*/
int
connect_ip(char *ip, int port)
{
	struct sockaddr_in addr;
	int fd;

	/* server address */
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(ip, &(addr.sin_addr));

	/* get new socket */
	if ((fd=socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Client: socket");
		return -1;
	}
	
	#ifdef DEBUG
	printf("\nClient: Connecting...\n");
	#endif

	/* connect to server */
	if (connect(fd,(struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
		perror("Client: connect");
		close(fd);
		return -1;
	}

	return fd;
}
