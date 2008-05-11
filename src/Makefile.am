AM_CFLAGS = -Wall -Werror

lib_LTLIBRARIES = libip-usbph.la

bin_PROGRAMS = ip-usbph

include_HEADERS = ip-usbph.h IP_USBPh

libip_usbph_la_SOURCES = \
			ip-usbph-font.c \
			ip-usbph.c ip-usbph.h

libip_usbph_la_CFLAGS = $(IP_USBPH_CFLAGS)
libip_usbph_la_LIBADD = $(IP_USBPH_LIBS)

ip_usbph_SOURCES = main.c argv.c argv.h
ip_usbph_LDADD = libip-usbph.la
