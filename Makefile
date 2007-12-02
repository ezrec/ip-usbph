all: test

libip-usbph.a: ip-usbph.c ip-usbph-font.c ip-usbph.h
	$(CC) -c ip-usbph.c
	$(CC) -c ip-usbph-font.c
	$(AR) rc libip-usbph.a ip-usbph.o ip-usbph-font.o

test: libip-usbph.a test.c
	$(CC) -o test test.c -L. -lip-usbph -lusb
