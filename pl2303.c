#include <stdio.h>
#include <stdlib.h>
/* According to POSIX.1-2001 */
#include <sys/select.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <libusb.h>
#include <getopt.h>


#define _GNU_SOURCE
#include <getopt.h>


#define I_VENDOR_NUM        0x067b
#define I_PRODUCT_NUM       0x2303


#define VENDOR_READ_REQUEST_TYPE	0xc0
#define VENDOR_READ_REQUEST			0x01

#define VENDOR_WRITE_REQUEST_TYPE	0x40
#define VENDOR_WRITE_REQUEST		0x01

void handle_error(int);

int get_device_vid()
{
	return I_VENDOR_NUM;
}

int get_device_pid()
{
	return I_PRODUCT_NUM;
}

//
// http://zoobab.wdfiles.com/local--files/pl2303hxd-gpio/pl2303-gpio-sysfs.patch
// http://www.zoobab.com/pl2303hxd-gpio
//

/* Get current GPIO register from PL2303 */
char gpio_read_reg(libusb_device_handle *h, int gpio)
{
	unsigned char buf;
	int bytes = libusb_control_transfer(
		h,             // handle obtained with usb_open()
		VENDOR_READ_REQUEST_TYPE, // bRequestType
		VENDOR_READ_REQUEST,      // bRequest
		(gpio <= 1) ? 0x0081 : 0x8d8d,   // wValue
		0,              // wIndex
		&buf,             // pointer to destination buffer
		1,  // wLength
		1000
		);
	handle_error(bytes);
	return buf;
}

void gpio_write_reg(libusb_device_handle *h, unsigned char reg, int gpio, int dir_value_ctrl)
{
	int bytes = libusb_control_transfer(
		h,             // handle obtained with usb_open()
		VENDOR_WRITE_REQUEST_TYPE, // bRequestType
		VENDOR_WRITE_REQUEST,      // bRequest
		(gpio <= 1) ? 0x0001 : (0x0c0c | dir_value_ctrl),   // wValue
		reg,              // wIndex
		0,             // pointer to destination buffer
		0,  // wLength
		1000
		);
	handle_error(bytes);
}

int gpio_dir_shift(int gpio) {
	if (gpio == 0)
		return 4;
	if (gpio == 1)
		return 5;
	return 4; /* default to 0 */
}

int gpio_val_shift(int gpio) {
	if (gpio == 0)
		return 6;
	if (gpio == 1)
		return 7;
	if (gpio == 2)
		return 0;
	if (gpio == 3)
		return 1;
	return 6; /* default to 0 */
}


void gpio_out(libusb_device_handle *h, int gpio, int value)
{
	unsigned char reg = gpio_read_reg(h, gpio);
 	int shift_val = gpio_val_shift(gpio);

	if (gpio == 2) {
// Something should be done to switch the pin mode back to output.
//		reg |= (0x03);
		reg |= (0x01);
	}
	else if (gpio == 3) {
		reg |= (0x0c);
	}
	else {
		reg |= (1 << gpio_dir_shift(gpio));
	}

	reg &= ~(1 << shift_val);
	reg |= (value << shift_val);
	gpio_write_reg(h, reg, gpio, 0x0101);
}

void gpio_in(libusb_device_handle *h, int gpio, int pullup)
{
	unsigned char reg = gpio_read_reg(h, gpio);
 	int shift_val = gpio_val_shift(gpio);

// Not sure what's wrong. When 2 or 3 is set to -in, both become -in.
	if (gpio == 2) {
		reg &= ~(0x03);
	}
	else if (gpio == 3) {
		reg &= ~(0x0c);
	}
	else {
		reg &= ~(1 << gpio_dir_shift(gpio));
	}

	reg &= ~(1 << shift_val);
	reg |= (pullup << shift_val);
	gpio_write_reg(h, reg, gpio, 0x0000);
}

int gpio_read(libusb_device_handle *h, int gpio)
{
	unsigned char r = gpio_read_reg(h, gpio);

	int shift = gpio_val_shift(gpio);
	return (r & (1<<shift));
}
