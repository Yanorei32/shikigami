#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void applyState(int channel, int angle) {
	if ( !(0 <= angle && angle <= 180) ) {
		std::cerr << "Angle value is out of range: " << angle;
		return;
	}
	std::cout << "Chan: " << channel << ", Angle: " << angle << std::endl;
}

int main (int, char**) {
	int sock, sock0;
	unsigned char buf[128];
	struct sockaddr_in addr;
	struct sockaddr_in client;
	auto len = sizeof client;

	// IPv4 STREAM TCP
	if ((sock0 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		std::cerr << "socket() returned -1: errno: ";
		std::cerr << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	// IPv4 0.0.0.0:12345
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock0, (struct sockaddr *)&addr, sizeof addr) < 0) {
		std::cerr << "bind() returned -1: errno: ";
		std::cerr << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	if (listen(sock0, 5) == -1) {
		std::cerr << "listen() returned -1: errno: ";
		std::cerr << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	int recv_bytes;
	for (;;) {
		unsigned char firstByte = 0x80;
		std::cout << "Waiting for new connection..." << std::endl;
		sock = accept(sock0, (struct sockaddr *)&client, &len);

		std::cout << "Socket is openned" << std::endl;

		for (;;) {
			recv_bytes = recv(sock, buf, sizeof buf, 0);

			if (recv_bytes == -1) {
				std::cerr << "recv() returned -1: errno: ";
				std::cerr << strerror(errno) << std::endl;
				break;
			}

			if (recv_bytes == 0) {
				std::cout << "Socket is closed" << std::endl;
				break;
			}

			for (int i = 0;i < recv_bytes;++i) {
				if (buf[i] & 0x80) {
					// illegal input (first byte is not initialized)
					if (firstByte & 0x80) continue;

					// used flag
					firstByte |= 0x80;

					int channel = (firstByte & 0x7c) >> 2;
					int angle = ((((int)firstByte) & 0x03) << 7) | (buf[i] & 0x7f);

					applyState(channel, angle);
				} else {
					firstByte = buf[i];
				}
			}
		}

		if (close(sock) == -1) {
			std::cerr << "close() returned -1: errno: ";
			std::cerr << strerror(errno) << std::endl;
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

