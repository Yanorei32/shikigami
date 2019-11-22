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

#include "../../../lib/pca9685v2.hpp"

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

	double tune;
	for (;;) {
		std::cout << "finetuning> ";
		std::cin >> tune;

		if (!std::cin.fail())
			break;

		std::cin.clear();
		std::cin.ignore(1024, '\n');
	}

	std::cout << "set tune=" << tune<< std::endl;

	auto drv1 = new PCA9685(fd, tune);
	drv1->init();

	double freq;
	for (;;) {
		std::cout << "frequency> ";
		std::cin >> freq;

		if (!std::cin.fail() && PCA9685::min_pwm_freq <= freq && freq <= PCA9685::max_pwm_freq)
			break;

		std::cin.clear();
		std::cin.ignore(1024, '\n');
	}

	std::cout << "set f=" << freq << std::endl;
	freq = drv1->set_pwm_freq(freq);

	double pulse;
	for (;;) {
		for (;;) {
			std::cout << "pulse> ";
			std::cin >> pulse;

			if (!std::cin.fail() && PCA9685::min_pwm_freq <= freq && freq <= PCA9685::max_pwm_freq)
				break;

			std::cin.clear();
			std::cin.ignore(1024, '\n');
		}

		std::cout << "set t=" << pulse << std::endl;

		drv1->set_pulse_us(PCA9685::channel_all, pulse, freq);
	}

	return EXIT_SUCCESS;
}


