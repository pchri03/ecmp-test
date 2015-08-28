#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifndef IP_PMTUDISC_DONT
#define IP_PMTUDISC_DONT 0
#endif

#ifndef IP_PMTUDISC_DO
#define IP_PMTUDISC_DO 2
#endif

#ifndef IP_PMTU_DISCOVER
#define IP_MTU_DISCOVER 10
#endif

int main(int argc, char **argv)
{
	if (argc < 4) {
		fputs("Usage: ecmp-test SRC-PORT IP [frag|no-frag]\n", stderr);
		return EXIT_FAILURE;
	}

	int port = atoi(argv[1]);
	if (port <= 0 || port > 65535) {
		fprintf(stderr, "Invalid port number: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		fprintf(stderr, "Error creating socket: %m\n");
		return EXIT_FAILURE;
	}

	int val = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

	if (strcmp(argv[3], "frag") == 0) {
		val = IP_PMTUDISC_DONT;
	} else if (strcmp(argv[3], "no-frag") == 0) {
		val = IP_PMTUDISC_DO;
	} else {
		fprintf(stderr, "Invalid argument: %s\n", argv[3]);
		return EXIT_FAILURE;
	}
	setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
		fprintf(stderr, "Error binding to address: %m\n");
		return EXIT_FAILURE;
	}

	addr.sin_port = htons(80);
	if (inet_pton(AF_INET, argv[2], &addr.sin_addr) != 1) {
		fprintf(stderr, "Invalid IP address: %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
		fprintf(stderr, "Error connecting to host: %m\n");
		return EXIT_FAILURE;
	}

	static const char request[] = "GET /zero.bin HTTP/1.0\r\n\r\n";
	if (write(sockfd, request, sizeof(request) - 1) == -1) {
		fprintf(stderr, "Error sending request: %m\n");
		return EXIT_FAILURE;
	}

	printf("Receiving: ");
	fflush(stdout);

	char buffer[32768];
	int cnt;
	while ((cnt = read(sockfd, buffer, sizeof(buffer))) > 0) {
		fputc('.', stdout);
		fflush(stdout);
	}

	puts("");

	if (cnt != 0) {
		fprintf(stderr, "Error receiving data: %m\n");
		return EXIT_FAILURE;
	}

	close(sockfd);

	return EXIT_SUCCESS;
}
