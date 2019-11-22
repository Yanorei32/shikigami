#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../../lib/pca9685v2.hpp"

#define I2C_DEVICE		"/dev/i2c-1"
#define PCA9685_ADDR	0x40

void applyAngle(PCA9685 *drv, double freq, unsigned int channel, unsigned int angle) {
	static constexpr auto min_pulse_width = 350;
	static constexpr auto max_pulse_width = 2500;
	static constexpr auto max_angle = 200;
	
	if ( !(angle <= max_angle) ) {
		std::cerr << "Angle value is out of range: " << angle;
		return;
	}

	auto pulse_width = min_pulse_width + (max_pulse_width-min_pulse_width)*(((double)angle) / max_angle);

	std::cout << "Chan: " << channel << ", Angle: " << angle << std::endl;

	drv->set_pulse_us(channel, pulse_width, freq);
}

int main (int, char**) {
	static constexpr auto finetuning = 0.9578;
	static constexpr auto pwm_freq = 50;
	int sock, sock0;
	unsigned char buf[128];
	struct sockaddr_in addr;
	struct sockaddr_in client;
	auto len = sizeof client;
	int fd;

	if ((fd = open(I2C_DEVICE, O_RDWR)) == -1) {
		std::cerr << "open() returned -1: errno: ";
		std::cerr << strerror(errno) << std::endl;

		return EXIT_FAILURE;
	}

	if (ioctl(fd, I2C_SLAVE, PCA9685_ADDR) == -1) {
		std::cerr << "ioctl() returned -1: errno: ";
		std::cerr << strerror(errno) << std::endl;

		return EXIT_FAILURE;
	}

	auto drv1 = new PCA9685(fd, finetuning);
	drv1->init();
	auto freq = drv1->set_pwm_freq(pwm_freq);

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

					applyAngle(drv1, freq, channel, angle);
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

