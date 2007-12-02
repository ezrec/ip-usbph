/*
 * $Id$
 *
 * Copyright 2006, Jason McMullan
 * Author: Jason McMullan <jason.mcmullan@gmail.com>
 *
 */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "ip-usbph.h"

void symbol_test(struct ip_usbph *ph)
{
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_DOWN, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_UP, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_SAT, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_COLON, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_FRI, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_THU, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_M_AND_D, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_TUE, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_WED, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_MON, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_SUN, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_OUT, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_IN, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_NEW, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_MUTE, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_LOCK, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_MAN, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_BALANCE, 1);
	ip_usbph_symbol(ph, IP_USBPH_SYMBOL_DECIMAL, 1);
	ip_usbph_flush(ph);
}

void digit_test(struct ip_usbph *ph)
{
	/* 8 test */
	ip_usbph_top_digit(ph, 0, ip_usbph_font_digit('1'));
	ip_usbph_top_digit(ph, 1, ip_usbph_font_digit('8'));
	ip_usbph_top_digit(ph, 2, ip_usbph_font_digit('8'));
	ip_usbph_top_digit(ph, 3, ip_usbph_font_digit('1'));
	ip_usbph_top_digit(ph, 4, ip_usbph_font_digit('8'));
	ip_usbph_top_digit(ph, 5, ip_usbph_font_digit('8'));
	ip_usbph_top_digit(ph, 6, ip_usbph_font_digit('8'));
	ip_usbph_top_digit(ph, 7, ip_usbph_font_digit('8'));
	ip_usbph_top_digit(ph, 8, ip_usbph_font_digit('8'));
	ip_usbph_top_digit(ph, 9, ip_usbph_font_digit('8'));
	ip_usbph_top_digit(ph, 10, ip_usbph_font_digit('8'));
	ip_usbph_flush(ph);
}

int main(int argc, char **argv)
{
	struct ip_usbph *ph;
	char buff[1024];
	uint8_t cmd[8] = { 0 };
	uint64_t shifty = 0;

	ph = ip_usbph_acquire(0);
	assert(ph != NULL);
	ip_usbph_init(ph);

	ip_usbph_clear(ph);
	ip_usbph_backlight(ph);

	symbol_test(ph);

	digit_test(ph);

#if 1
	while (fgets(buff, sizeof(buff)-1, stdin) != NULL) {
		char *cp, *next;
		int i;
		unsigned long v;

		i = 0;
		next = buff;
		do {
			cp = next;
			v = strtoul(cp, &next, 16);
			if (cp != next) {
				shifty = 1;
				cmd[i++] = v;
			}
		} while (i < 8 && cp != next);

		ip_usbph_raw(ph, cmd);

		for (i = 3; i < 8; i++) {
			cmd[i] = (~(shifty) >> ((i - 3)*8)) & 0xff;
		}
		for (i = 0; i < 8; i++) {
			printf("%02X%s", cmd[i], i==7 ? "] " : " ");
		}
		shifty <<= 1;
		fflush(stdout);
	}
#endif

	sleep(1000);

	ip_usbph_release(ph);

	return 0;
}
