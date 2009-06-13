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
#include <poll.h>

#include "ip-usbph.h"

const char *keymap[0x20] = {
	[IP_USBPH_KEY_0] = "0",
	[IP_USBPH_KEY_1] = "1",
	[IP_USBPH_KEY_2] = "2",
	[IP_USBPH_KEY_3] = "3",
	[IP_USBPH_KEY_4] = "4",
	[IP_USBPH_KEY_5] = "5",
	[IP_USBPH_KEY_6] = "6",
	[IP_USBPH_KEY_7] = "7",
	[IP_USBPH_KEY_8] = "8",
	[IP_USBPH_KEY_9] = "9",
	[IP_USBPH_KEY_YES] = "YES",
	[IP_USBPH_KEY_NO] = "NO",
	[IP_USBPH_KEY_VOL_UP] = "VOL+",
	[IP_USBPH_KEY_VOL_DOWN] = "VOL-",
	[IP_USBPH_KEY_UP] = "UP",
	[IP_USBPH_KEY_DOWN] = "DOWN",
	[IP_USBPH_KEY_S] = "S",
	[IP_USBPH_KEY_C] = "C",
	[IP_USBPH_KEY_ASTERISK] = "*",
	[IP_USBPH_KEY_HASH] = "#",
};
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

void top_char_test(struct ip_usbph *ph)
{
	int i;

	/* Char 255 test */
	for (i = 0; i < 8; i++) {
		ip_usbph_top_char(ph, i, ip_usbph_font_char(255));
	}
	ip_usbph_flush(ph);

	sleep(1);

	for (i = 0; i < 8; i++) {
		ip_usbph_top_char(ph, i, ip_usbph_font_char(i + 'A'));
	}
	ip_usbph_flush(ph);
}

void bot_char_test(struct ip_usbph *ph)
{
	int i;

	/* Char 255 test */
	for (i = 0; i < 4; i++) {
		ip_usbph_bot_char(ph, i, ip_usbph_font_char(255));
	}
	ip_usbph_flush(ph);

	sleep(1);

	for (i = 0; i < 4; i++) {
		ip_usbph_bot_char(ph, i, ip_usbph_font_char(i + 'a'));
	}
	ip_usbph_flush(ph);
}


int main(int argc, char **argv)
{
	struct ip_usbph *ph;
	char buff[1024];
	uint8_t cmd[8] = { 0 };
	uint64_t shifty = 0;
	int fd, err;

	ph = ip_usbph_acquire(0);
	assert(ph != NULL);

	ip_usbph_clear(ph);
	ip_usbph_backlight(ph);

	symbol_test(ph);

	digit_test(ph);

	top_char_test(ph);

	bot_char_test(ph);

	fd = ip_usbph_key_fd(ph);
	if (fd < 0) {
		printf("Could not talk to the phone's keypad. Skipping keypad test.\n");
	} else {
		struct pollfd fds[1] = {
			{ .fd = fd, .events = POLLIN | POLLERR }
		};
		int zeros = 0;

		printf("Press some keys. Press \"000\" to exit.\n");

		while ((err = poll(fds, 1, -1)) == 1) {
			uint16_t key;

			if (fds[0].revents & POLLERR) {
				break;
			}

			key = ip_usbph_key_get(ph);
			if (key == IP_USBPH_KEY_INVALID) {
				printf("Crap. We broke the keypad support.\n");
				break;
			}

			printf("Key: %s (%s)\n", keymap[key & 0x1f], (key & IP_USBPH_KEY_PRESSED) ? "Down" : "Up");

			if ((key & IP_USBPH_KEY_PRESSED) == 0) {
				if (key == IP_USBPH_KEY_0) {
					zeros++;
				} else {
					zeros = 0;
				}
			}

			if (zeros == 3) {
				break;
			}
		}
	}

	ip_usbph_release(ph);

	return 0;
}
