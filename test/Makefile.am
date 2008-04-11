
noinst_PROGRAMS = test_c test_cpp

test_c_SOURCES = test_c.c

test_c_CFLAGS = -I$(top_srcdir)/src $(IP_USBPH_CFLAGS)
test_c_LDADD = ../src/libip-usbph.la $(IP_USBPH_LIBS)

test_cpp_SOURCES = test_cpp.cpp

test_cpp_CPPFLAGS = -I$(top_srcdir)/src $(IP_USBPH_CFLAGS)
test_cpp_LDADD = ../src/libip-usbph.la $(IP_USBPH_LIBS)
