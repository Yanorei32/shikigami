#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define CHAN_COUNT 32
#define MAX_ANGLE 180

int main (int, char**) {
	struct sockaddr_in server;
	int sock;
	int chan, angle;

	unsigned char buf[2];

	for (;;) {
		std::cout << "Channel: " << std::flush;
		std::cin >> chan;

		if (!std::cin.fail() && 0 <= chan && chan < CHAN_COUNT)
			break;

		std::cin.clear();
		std::cin.ignore(1024, '\n');
	}

	for (;;) {
		std::cout << "Angle: " << std::flush;
		std::cin >> angle;

		if (!std::cin.fail() && 0 <= angle && angle <= MAX_ANGLE)
			break;

		std::cin.clear();
		std::cin.ignore(1024, '\n');
	}

	buf[0] = (angle & 0x180) >> 7 | chan << 2;
	buf[1] = (angle & 0x7f) | 0x80;
	printf("%02X %02X\n", buf[0], buf[1]);

	// IPv4 STREAM TCP
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		std::cerr << "socket() returned -1: errno: ";
		std::cerr << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(12345);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	// Connection
	if (connect(sock, (struct sockaddr *)&server, sizeof server) == -1) {
		std::cerr << "connect() returned -1: errno: ";
		std::cerr << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	if (send(sock, buf, sizeof buf, 0) == -1) {
		std::cerr << "send() returned -1: errno: ";
		std::cerr << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	if (close(sock) == -1) {
		std::cerr << "close() returned -1: errno: ";
		std::cerr << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

