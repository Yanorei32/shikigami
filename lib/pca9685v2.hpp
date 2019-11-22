#include <cmath>
#include <iostream>
extern "C" {
#include <i2c/smbus.h>
}
#include <linux/types.h>
#include <ratio>
#include <unistd.h>

class PCA9685 {
	private:
		constexpr static __u8 addr_mode1			= 0x00;
		constexpr static __u8 flag_mode1_sleep		= 0x10;
		constexpr static __u8 flag_mode1_restart	= 0x80;
		constexpr static __u8 flag_mode1_allcall	= 0x11;
		constexpr static __u8 addr_mode2			= 0x01;
		constexpr static __u8 flag_mode2_outdrv		= 0x04;
		constexpr static __u8 addr_prescale			= 0xFE;
		constexpr static __u8 addr_chan0_on_l		= 0x06;
		constexpr static __u8 addr_chan0_on_h		= 0x07;
		constexpr static __u8 addr_chan0_off_l		= 0x08;
		constexpr static __u8 addr_chan0_off_h		= 0x09;
		constexpr static __u8 addr_chan0_all_diff	= 0xF4;

		int fd;
		double freq_finetuning;

	public:
		constexpr static auto channel_all		= -1;
		constexpr static auto osc_freq			= 25 * (std::mega::num / std::mega::den);
		constexpr static auto min_pwm_freq		= 24;
		constexpr static auto max_pwm_freq		= 1526;
		constexpr static auto resolution		= 4096;
		constexpr static auto osc_stab_delay	= 500;

		PCA9685(int fd, double freq_finetuning = 1.0) {
			this->fd = fd;
			this->freq_finetuning = freq_finetuning;
		}

		void init() {
			this->set_pulse(channel_all, 0, 0);

			i2c_smbus_write_byte_data(
				this->fd, addr_mode2, flag_mode2_outdrv
			);

			i2c_smbus_write_byte_data(
				this->fd, addr_mode1, flag_mode1_allcall
			);

			usleep(osc_stab_delay);

			i2c_smbus_write_byte_data(
				this->fd,
				addr_mode1,
				i2c_smbus_read_byte_data(
					this->fd,
					addr_mode1
				) & ~flag_mode1_sleep
			);

			usleep(osc_stab_delay);
		}

		double set_pwm_freq(unsigned int freq) {
			__u8 prescale_reg;

			auto mode1 = i2c_smbus_read_byte_data(
				this->fd,
				addr_mode1
			);

			i2c_smbus_write_byte_data(
				this->fd,
				addr_mode1,
				(mode1 & 0x7f) | flag_mode1_sleep
			);

			i2c_smbus_write_byte_data(
				this->fd,
				addr_prescale,
				prescale_reg = std::round(
					( (double)osc_freq / (resolution * freq) )
				) - 1
			);

			i2c_smbus_write_byte_data(
				this->fd,
				addr_mode1,
				mode1
			);

			usleep(osc_stab_delay);

			i2c_smbus_write_byte_data(
				this->fd,
				addr_mode1,
				mode1 | flag_mode1_restart
			);

			return (double)osc_freq / ((prescale_reg+1) * resolution);
		}

		void set_pulse(
			int chan,
			unsigned int on,
			unsigned int off
		) {
			auto addr_shift = chan == channel_all ? addr_chan0_all_diff : chan * 4;

			i2c_smbus_write_byte_data(
				this->fd, addr_shift + addr_chan0_on_l, on & 0xFF
			);

			i2c_smbus_write_byte_data(
				this->fd, addr_shift + addr_chan0_on_h, on >> 8
			);

			i2c_smbus_write_byte_data(
				this->fd, addr_shift + addr_chan0_off_l, off & 0xFF
			);

			i2c_smbus_write_byte_data(
				this->fd, addr_shift + addr_chan0_off_h, off >> 8
			);
		}

		void set_pulse_us(
			int chan,
			double pulse_us,
			double pwm_freq
		) {
			double step_us =
				(std::micro::den / std::micro::num) / pwm_freq / resolution;

			this->set_pulse(
				chan,
				0,
				(unsigned int)(
					std::round( (pulse_us * freq_finetuning) / step_us )
				)
			);
		}
};

