#include <cmath>
#include <iostream>
extern "C" {
#include <i2c/smbus.h>
}
#include <unistd.h>

#define ADDR_MODE1			0x00
#define FLAG_MODE1_SLEEP	0x10
#define FLAG_MODE1_RESTART	0x80
#define FLAG_MODE1_ALLCALL	0x01
#define ADDR_MODE2			0x01
#define FLAG_MODE2_OUTDRV	0x04
#define ADDR_PRESCALE		0xFE
#define ADDR_CHAN0_ON_L		0x06
#define ADDR_CHAN0_ON_H		0x07
#define ADDR_CHAN0_OFF_L	0x08
#define ADDR_CHAN0_OFF_H	0x09
#define ADDR_CHAN0_ALL_DIFF	0xF4

#define CHANNEL_ALL			-1

#define OSC_FREQ			25*1000*1000
#define RESOLUTION			4096
#define MAXIMUM_PWM_FREQ	1526
#define MINIMUM_PWM_FREQ	24

void pca9685_set_pwm(int fd, int channel, unsigned int on, unsigned int off) {
	int addr_shift = channel == CHANNEL_ALL ? ADDR_CHAN0_ALL_DIFF : channel * 4;

	i2c_smbus_write_byte_data(fd, addr_shift + ADDR_CHAN0_ON_L, on & 0xFF);
	i2c_smbus_write_byte_data(fd, addr_shift + ADDR_CHAN0_ON_H, on >> 8);
	i2c_smbus_write_byte_data(fd, addr_shift + ADDR_CHAN0_OFF_L, off & 0xFF);
	i2c_smbus_write_byte_data(fd, addr_shift + ADDR_CHAN0_OFF_H, off >> 8);
}

void pca9685_init(int fd) {
	pca9685_set_pwm(fd, CHANNEL_ALL, 0, 0);

	i2c_smbus_write_byte_data(fd, ADDR_MODE2, FLAG_MODE2_OUTDRV);
	i2c_smbus_write_byte_data(fd, ADDR_MODE1, FLAG_MODE1_ALLCALL);

	usleep(500);

	i2c_smbus_write_byte_data(
		fd,
		ADDR_MODE1,
		i2c_smbus_read_byte_data(fd, ADDR_MODE1) & ~FLAG_MODE1_SLEEP
	);

	usleep(500);
}

void pca9685_set_pwm_freq_with_finetune(int fd, unsigned int freq, int tune) {
	auto mode1 = i2c_smbus_read_byte_data(fd, ADDR_MODE1);
	unsigned char prescale_reg_value;

	i2c_smbus_write_byte_data(fd, ADDR_MODE1, (mode1 & 0x7F) | FLAG_MODE1_SLEEP);

	i2c_smbus_write_byte_data(
		fd,
		ADDR_PRESCALE,
		prescale_reg_value = std::round( ((double)OSC_FREQ) / (RESOLUTION * freq) ) - 1 + tune
	);

#ifdef SHOW_PCA9685_TRUE_FREQ
	auto true_freq = ( (double)OSC_FREQ ) / ( (prescale_reg_value + 1) * RESOLUTION );

	std::cerr << "hw freq: " << true_freq << " Hz" << std::endl;
#endif

	i2c_smbus_write_byte_data(fd, ADDR_MODE1, mode1);

	usleep(500);

	i2c_smbus_write_byte_data(fd, ADDR_MODE1, mode1 | FLAG_MODE1_RESTART);
}

void pca9685_set_pwm_freq(int fd, unsigned int freq) {
	pca9685_set_pwm_freq_with_finetune(fd, freq, 0);
}

void pca9685_set_pulse_us(int fd, unsigned int pwm_freq, int channel, double pulse_us) {
	double step_us = (double)1*1000*1000 / pwm_freq / RESOLUTION;

	pca9685_set_pwm(
		fd, channel, 0,
		(unsigned int)( std::round( pulse_us / step_us ) )
	);
}

