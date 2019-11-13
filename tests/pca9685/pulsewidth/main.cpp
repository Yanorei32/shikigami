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

	const auto pwm_freq = 50;

	pca9685_init(fd);
	pca9685_set_pwm_freq(fd, pwm_freq);

	int pulse_us;
	for (;;) {
		for (;;) {
			std::cout << "pulse_us> ";
			std::cin >> pulse_us;

			if (!std::cin.fail() && 0 <= pulse_us && pulse_us <= (1*1000*1000)/pwm_freq)
				break;

			std::cin.clear();
			std::cin.ignore(1024, '\n');
		}

		std::cout << "set pw=" << pulse_us << std::endl;
		pca9685_set_pulse_us(fd, pwm_freq, 0, pulse_us);
	}

	return EXIT_SUCCESS;
}

