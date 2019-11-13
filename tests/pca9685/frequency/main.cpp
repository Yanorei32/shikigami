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

#include "../../../lib/pca9685.hpp"

#define I2C_DEVICE		"/dev/i2c-1"
#define PCA9685_ADDR	0x40

int main(int, char**) {
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

	pca9685_init(fd);

	int freq;
	for (;;) {
		for (;;) {
			std::cout << "frequency> ";
			std::cin >> freq;

			if (!std::cin.fail() && MINIMUM_PWM_FREQ <= freq && freq <= MAXIMUM_PWM_FREQ)
				break;

			std::cin.clear();
			std::cin.ignore(1024, '\n');
		}

		std::cout << "set f=" << freq << std::endl;
		pca9685_set_pwm_freq(fd, freq);
		pca9685_set_pwm(fd, CHANNEL_ALL, 0, RESOLUTION / 2);
	}

	return EXIT_SUCCESS;
}

