/*
 * $Id$
 *
 * Copyright 2006, Jason McMullan
 * Author: Jason McMullan <jason.mcmullan@gmail.com>
 *
 */

#ifndef IP_USBPH_DEF_H
#define IP_USBPH_DEF_H

/* Symbols
 */
typedef enum {
	IP_USBPH_SYMBOL_DOWN = 1,
	IP_USBPH_SYMBOL_UP,
	IP_USBPH_SYMBOL_SAT,
	IP_USBPH_SYMBOL_COLON,
	IP_USBPH_SYMBOL_FRI,
	IP_USBPH_SYMBOL_THU,
	IP_USBPH_SYMBOL_M_AND_D,
	IP_USBPH_SYMBOL_TUE,
	IP_USBPH_SYMBOL_WED,
	IP_USBPH_SYMBOL_MON,
	IP_USBPH_SYMBOL_SUN,
	IP_USBPH_SYMBOL_OUT,
	IP_USBPH_SYMBOL_IN,
	IP_USBPH_SYMBOL_NEW,
	IP_USBPH_SYMBOL_MUTE,
	IP_USBPH_SYMBOL_LOCK,
	IP_USBPH_SYMBOL_MAN,
	IP_USBPH_SYMBOL_BALANCE,
	IP_USBPH_SYMBOL_DECIMAL,
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
 * Digits 1 and 4 are 'entire' segments (representing "1" only)
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

#define IP_USBPH_KEY_INVALID	(0)
#define IP_USBPH_KEY_PRESSED	(1 << 15)

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

#endif /* IP_USBPH_DEF_H */
