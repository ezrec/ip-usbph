/*
 * Copyright 2006-2009, Jason McMullan
 * Author: Jason McMullan <jason.mcmullan@gmail.com>
 *
 * Licensed under the LGPL v2
 */

#ifndef IP_USBPH_H
#define IP_USBPH_H

#include <stdint.h>

/* Symbols
 */
typedef enum {
	IP_USBPH_SYMBOL_NEW,	/* 'NEW' in upper left */
	IP_USBPH_SYMBOL_SUN,	/* 'SUN' on top row */
	IP_USBPH_SYMBOL_MON,	/* 'MON' on top row */
	IP_USBPH_SYMBOL_TUE,	/* 'TUE' on top row */
	IP_USBPH_SYMBOL_WED,	/* 'WED' on top row */
	IP_USBPH_SYMBOL_THU,	/* 'THU' on top row */
	IP_USBPH_SYMBOL_FRI,	/* 'FRI' on top row */
	IP_USBPH_SYMBOL_SAT,	/* 'SAT' on top row */
	IP_USBPH_SYMBOL_IN,	/* 'IN'  (below 'NEW', upper left) */
	IP_USBPH_SYMBOL_OUT,	/* 'OUT' (below 'IN', upper left) */
	IP_USBPH_SYMBOL_M_AND_D,/* Month and Day '-' separators, digit row */
	IP_USBPH_SYMBOL_COLON,	/* Hour/Minute ':' separator, digit row */
	IP_USBPH_SYMBOL_UP,	/* Arrow up, right of top alpha row */
	IP_USBPH_SYMBOL_DOWN,	/* Arrow down, right of top alpha row */
	IP_USBPH_SYMBOL_BALANCE,/* 'Balance', above the bottom alpha row */
	IP_USBPH_SYMBOL_MAN,	/* Picture of a person, bottom left */
	IP_USBPH_SYMBOL_LOCK,	/* Picture of a lock, right of person image */
	IP_USBPH_SYMBOL_MUTE,	/* Mute symbol, right of the lock image */
	IP_USBPH_SYMBOL_DECIMAL,/* Decimal '.' separator, bottom alpha row */
} ip_usbph_sym;

/*
 * Character segments. 'OR' together to make a character.
 *
 *      ------T------
 *     | \    |    / |
 *    TL TLX TC  TRX TR
 *     |    \ | /    |
 *      --LC-- --RC--
 *     |    / | \    |
 *    BL BLX BC  BRX BR
 *     | /    |    \ |
 *      ------B------
 *
 * Digit segments.
 *
 *      -----T-----
 *     |           |
 *    TL          TR
 *     |           |
 *      -----M-----
 *     |           |
 *    BL          BR
 *     |           |
 *      -----B-----
 *
 * Digits 0 and 3 are 'entire' segments (representing "1" only)
 *
 *     |
 *     |
 *     |
 *     E
 *     |
 *     |
 *     |
 *
 */
#define IP_USBPH_SEG_T		(1 << 0)
#define IP_USBPH_SEG_B		(1 << 1)
#define IP_USBPH_SEG_TL		(1 << 2)
#define IP_USBPH_SEG_TR		(1 << 3)
#define IP_USBPH_SEG_BL		(1 << 4)
#define IP_USBPH_SEG_BR		(1 << 5)
#define IP_USBPH_SEG_M      	(1 << 6)	/* Only used on Digits */
#define IP_USBPH_SEG_E		(1 << 7)	/* Only used on Entire Digits */

#define IP_USBPH_SEG_TC		(1 << 8)	/* The rest are for Chars only */
#define IP_USBPH_SEG_BC		(1 << 9)
#define IP_USBPH_SEG_LC		(1 << 10)
#define IP_USBPH_SEG_RC		(1 << 11)
#define IP_USBPH_SEG_TLX	(1 << 12)
#define IP_USBPH_SEG_TRX	(1 << 13)
#define IP_USBPH_SEG_BLX	(1 << 14)
#define IP_USBPH_SEG_BRX	(1 << 15)

/* Keycodes */

#define IP_USBPH_KEY_IDLE	(0x00)
#define IP_USBPH_KEY_ERROR	(0xff)
#define IP_USBPH_KEY_PRESSED	(0x20)

#define IP_USBPH_KEY_1		0x01
#define IP_USBPH_KEY_2		0x02
#define IP_USBPH_KEY_3		0x03
#define IP_USBPH_KEY_VOL_UP	0x04
#define IP_USBPH_KEY_4		0x05
#define IP_USBPH_KEY_5		0x06
#define IP_USBPH_KEY_6		0x07
#define IP_USBPH_KEY_7		0x09
#define IP_USBPH_KEY_VOL_DOWN	0x08
#define IP_USBPH_KEY_8		0x0a
#define IP_USBPH_KEY_9		0x0b
#define IP_USBPH_KEY_ASTERISK	0x0c
#define IP_USBPH_KEY_0		0x0e
#define IP_USBPH_KEY_HASH	0x0f
#define IP_USBPH_KEY_S		0x10
#define IP_USBPH_KEY_UP		0x11
#define IP_USBPH_KEY_DOWN	0x12
#define IP_USBPH_KEY_NO		0x13
#define IP_USBPH_KEY_YES	0x14
#define IP_USBPH_KEY_C		0x15

struct ip_usbph *ip_usbph_acquire(int index);
void ip_usbph_release(struct ip_usbph *ph);

/* Save state to a fd
 *
 * Returns length written
 */
int ip_usbph_state_save(struct ip_usbph *ph, int fd);

/* Restore state from a fd
 */
int ip_usbph_state_load(struct ip_usbph *ph, int fd);

/* Backlight control (enable for 7 seconds)
 */
int ip_usbph_backlight(struct ip_usbph *ph);

/* Clear screen 
 */
int ip_usbph_clear(struct ip_usbph *ph);

/* Set a symbol
 */
int ip_usbph_symbol(struct ip_usbph *ph, ip_usbph_sym sym, int is_on);

struct ip_usbph;
typedef uint8_t ip_usbph_digit;
typedef uint16_t ip_usbph_char;

/* Get digit mask for characters 
 * Valid: 0-9, a-f, A-F 
 */
ip_usbph_digit ip_usbph_font_digit(uint8_t c);
ip_usbph_char  ip_usbph_font_char(uint8_t c);

/*
 * Valid indexes are from 0 to 10.
 * Index 0 and 3 are 'forced ones', and will only display
 * a space or '1'
 */
#define IP_USBPH_TOP_DIGITS	11
int ip_usbph_top_digit(struct ip_usbph *ph, int index, ip_usbph_digit digit);

/*
 * Valid indexes are from 0 to 7
 */
#define IP_USBPH_TOP_CHARS	8
int ip_usbph_top_char(struct ip_usbph *ph, int index, ip_usbph_char ch);

#define IP_USBPH_BOT_CHARS	4
int ip_usbph_bot_char(struct ip_usbph *ph, int index, ip_usbph_char ch);

/*
 * Flush new characters to the display
 */
int ip_usbph_flush(struct ip_usbph *ph);

/* Get keycode and is-up mask for a key.
 */
uint8_t ip_usbph_key_get(struct ip_usbph *ph, int timeout_sec);

#endif /* IP_USBPH_H */
