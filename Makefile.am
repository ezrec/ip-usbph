ACLOCAL_AMFLAGS=-I m4

SUBDIRS = src doc test

install-data-hook: libip-usbph.pc
	$(INSTALL) -D -m 0644 libip-usbph.pc $(DESTDIR)$(libdir)/pkgconfig/libip-usbph.pc
