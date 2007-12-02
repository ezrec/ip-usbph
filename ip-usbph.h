/*
 * $Id$
 *
 * Copyright 2006, Jason McMullan
 * Author: Jason McMullan <jason.mcmullan@gmail.com>
 *
 */

#ifndef IP_USBPH_H
#define IP_USBPH_H

#include <stdint.h>

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

struct ip_usbph;
typedef uint8_t ip_usbph_digit;
typedef uint16_t ip_usbph_char;

/* Get digit mask for characters 
 * Valid: 0-9, a-f, A-F 
 */
ip_usbph_digit ip_usbph_font_digit(uint8_t c);
ip_usbph_char  ip_usbph_font_char(uint8_t c);

struct ip_usbph *ip_usbph_acquire(int index);
void ip_usbph_release(struct ip_usbph *ph);

int ip_usbph_init(struct ip_usbph *ph);

int ip_usbph_raw(struct ip_usbph *ph, const uint8_t cmd[8]);

int ip_usbph_symbol(struct ip_usbph *ph, ip_usbph_sym sym, int is_on);

/*
 * Valid indexes are from 0 to 10.
 * Index 0 and 3 are 'forced ones'
 */
int ip_usbph_top_digit(struct ip_usbph *ph, int index, ip_usbph_digit digit);

int ip_usbph_top_char(struct ip_usbph *ph, int index, ip_usbph_char ch);

int ip_usbph_flush(struct ip_usbph *ph);

#endif /* IP_USBPH_H */
