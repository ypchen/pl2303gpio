
LIBUSB_CFLAGS  ?= $(shell pkg-config --cflags libusb-1.0)
LIBUSB_LDFLAGS ?= $(shell pkg-config --libs libusb-1.0)

PREFIX?=/usr/local

all: pl2303gpio cp2103gpio

OBJS=usb.c main.c

pl2303gpio: $(OBJS) pl2303.c
	$(CC) $(CFLAGS) $(LIBUSB_CFLAGS) -Wall -Werror -o $(@) $(^) $(LDFLAGS) $(LIBUSB_LDFLAGS)

cp2103gpio: $(OBJS) cp2103.c
	$(CC) $(CFLAGS) $(LIBUSB_CFLAGS) -Wall -Werror -o $(@) $(^) $(LDFLAGS) $(LIBUSB_LDFLAGS)

clean:
	-rm pl2303gpio cp2103gpio

install: pl2303gpio cp2103gpio
	cp pl2303gpio $(PREFIX)/bin
	cp cp2103gpio $(PREFIX)/bin

install-rules:	
	cp 10-pl2303_cp210x_userspace.rules /etc/udev/rules.d
	udevadm control --reload-rules

install-scripts:
	cp extra/serverctl   $(PREFIX)/bin/
	cp extra/serverd.lua $(PREFIX)/bin/
	cp etc/serverd.conf  $(PREFIX)/etc/
