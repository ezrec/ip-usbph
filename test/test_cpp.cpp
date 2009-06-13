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
#include <unistd.h>

#include <IP_USBPh>

const static struct {
	uint16_t key;
	const char *value;
} keymap[] = {
	{ IP_USBPH_KEY_0, "0" },
	{ IP_USBPH_KEY_1, "1" },
	{ IP_USBPH_KEY_2, "2" },
	{ IP_USBPH_KEY_3, "3" },
	{ IP_USBPH_KEY_4, "4" },
	{ IP_USBPH_KEY_5, "5" },
	{ IP_USBPH_KEY_6, "6" },
	{ IP_USBPH_KEY_7, "7" },
	{ IP_USBPH_KEY_8, "8" },
	{ IP_USBPH_KEY_9, "9" },
	{ IP_USBPH_KEY_YES, "YES" },
	{ IP_USBPH_KEY_NO, "NO" },
	{ IP_USBPH_KEY_VOL_UP, "VOL+" },
	{ IP_USBPH_KEY_VOL_DOWN, "VOL-" },
	{ IP_USBPH_KEY_UP, "UP" },
	{ IP_USBPH_KEY_DOWN, "DOWN" },
	{ IP_USBPH_KEY_S, "S" },
	{ IP_USBPH_KEY_C, "C" },
	{ IP_USBPH_KEY_ASTERISK, "*" },
	{ IP_USBPH_KEY_HASH, "#" },
	{ IP_USBPH_KEY_INVALID, "Invalid" },
};

void symbol_test(IP_USBPh *ph)
{
	ph->symbol(IP_USBPH_SYMBOL_DOWN, 1);
	ph->symbol(IP_USBPH_SYMBOL_UP, 1);
	ph->symbol(IP_USBPH_SYMBOL_SAT, 1);
	ph->symbol(IP_USBPH_SYMBOL_COLON, 1);
	ph->symbol(IP_USBPH_SYMBOL_FRI, 1);
	ph->symbol(IP_USBPH_SYMBOL_THU, 1);
	ph->symbol(IP_USBPH_SYMBOL_M_AND_D, 1);
	ph->symbol(IP_USBPH_SYMBOL_TUE, 1);
	ph->symbol(IP_USBPH_SYMBOL_WED, 1);
	ph->symbol(IP_USBPH_SYMBOL_MON, 1);
	ph->symbol(IP_USBPH_SYMBOL_SUN, 1);
	ph->symbol(IP_USBPH_SYMBOL_OUT, 1);
	ph->symbol(IP_USBPH_SYMBOL_IN, 1);
	ph->symbol(IP_USBPH_SYMBOL_NEW, 1);
	ph->symbol(IP_USBPH_SYMBOL_MUTE, 1);
	ph->symbol(IP_USBPH_SYMBOL_LOCK, 1);
	ph->symbol(IP_USBPH_SYMBOL_MAN, 1);
	ph->symbol(IP_USBPH_SYMBOL_BALANCE, 1);
	ph->symbol(IP_USBPH_SYMBOL_DECIMAL, 1);
	ph->flush();
}

void digit_test(IP_USBPh *ph)
{
	/* 8 test */
	ph->top_digit(0, ph->font_digit('1'));
	ph->top_digit(1, ph->font_digit('8'));
	ph->top_digit(2, ph->font_digit('8'));
	ph->top_digit(3, ph->font_digit('1'));
	ph->top_digit(4, ph->font_digit('8'));
	ph->top_digit(5, ph->font_digit('8'));
	ph->top_digit(6, ph->font_digit('8'));
	ph->top_digit(7, ph->font_digit('8'));
	ph->top_digit(8, ph->font_digit('8'));
	ph->top_digit(9, ph->font_digit('8'));
	ph->top_digit(10, ph->font_digit('8'));

	ph->flush();
}

void top_char_test(IP_USBPh *ph)
{
	int i;

	/* Char 255 test */
	for (i = 0; i < 8; i++) {
		ph->top_char(i, ph->font_char(255));
	}
	ph->flush();

	sleep(1);

	for (i = 0; i < 8; i++) {
		ph->top_char(i, ph->font_char(i + 'A'));
	}
	ph->flush();
}

void bot_char_test(IP_USBPh *ph)
{
	int i;

	/* Char 255 test */
	for (i = 0; i < 4; i++) {
		ph->bot_char(i, ph->font_char(255));
	}
	ph->flush();

	sleep(1);

	for (i = 0; i < 4; i++) {
		ph->bot_char(i, ph->font_char(i + 'a'));
	}
	ph->flush();
}


int main(int argc, char **argv)
{
	IP_USBPh *ph;
	char buff[1024];
	uint8_t cmd[8] = { 0 };
	uint64_t shifty = 0;
	int fd, err;

	ph = new IP_USBPh(0);
	assert(ph != NULL);

	ph->clear();
	ph->backlight();

	symbol_test(ph);

	digit_test(ph);

	top_char_test(ph);

	bot_char_test(ph);

	fd = ph->key_fd();
	if (fd < 0) {
		printf("Could not talk to the phone's keypad. Skipping keypad test.\n");
	} else {
		struct pollfd fds[1] = {
			{ fd, POLLIN | POLLERR, 0 }
		};
		int zeros = 0;

		printf("Press some keys. Press \"000\" to exit.\n");

		while ((err = poll(fds, 1, -1)) == 1) {
			uint16_t key;
			int i;

			if (fds[0].revents & POLLERR) {
				break;
			}

			key = ph->key_get();
			if (key == IP_USBPH_KEY_INVALID) {
				printf("Crap. We broke the keypad support.\n");
				break;
			}

			for (i = 0; keymap[i].key != IP_USBPH_KEY_INVALID; i++) {
				if (keymap[i].key == (key & 0x1f))
					break;
			}
			printf("Key: %s (%s)\n", keymap[i].value, (key & IP_USBPH_KEY_PRESSED) ? "Down" : "Up");

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

	delete ph;

	return 0;
}
