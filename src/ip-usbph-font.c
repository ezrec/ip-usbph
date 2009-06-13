/*
 * Copyright 2006, Jason McMullan
 * Author: Jason McMullan <jason.mcmullan@gmail.com>
 *
 * Licensed under the LGPL v2
 */
#include <ctype.h>

#include "ip-usbph.h"

#define ARRAY_SIZE(x)	(sizeof(x)/sizeof(x[0]))

/* Shorthand definitions */
#define _T	IP_USBPH_SEG_T
#define _TL	IP_USBPH_SEG_TL
#define _TLX	IP_USBPH_SEG_TLX
#define _TRX	IP_USBPH_SEG_TRX
#define _TC	IP_USBPH_SEG_TC
#define _TR	IP_USBPH_SEG_TR
#define _LC	IP_USBPH_SEG_LC
#define _RC	IP_USBPH_SEG_RC
#define _BL	IP_USBPH_SEG_BL
#define _BLX	IP_USBPH_SEG_BLX
#define _BC	IP_USBPH_SEG_BC
#define _BRX	IP_USBPH_SEG_BRX
#define _BR	IP_USBPH_SEG_BR
#define _B	IP_USBPH_SEG_B
#define _M	IP_USBPH_SEG_M
#define _E	IP_USBPH_SEG_E

#define _L      (_TL | _BL)
#define _R      (_TR | _BR)

/* Get char mask
 *  * Valid: 
 *   */
static const ip_usbph_char font_char[256] = {
	[' '] = 0,
	['"'] = _TC | _TR,
	['#'] = _TC | _TR |
	        _LC | _RC |
	        _BC | _BR |
	        _B,
	['$'] = _T |
	        _TL | _TC | 
	        _LC | _RC |
	        _BC | _BR |
	        _B,
	['%'] = _TL | _TLX |
	        _TRX |
	        _LC | _RC |
	        _BLX |
	        _BRX | _BR,
	['\''] = _TRX,
	['*'] = _TLX | _TC | _TRX |
		_LC | _RC |
		_BLX | _BC | _BRX,
	['+'] = _TC | _LC | _RC | _B,
	['`'] = _TLX,
	['-'] = _LC | _RC,
	['/'] = _TRX | _BLX,
	['0'] = _T | _L | _R | _B | _BLX | _TRX,
	['1'] = _R | _E,
	['2'] = _T | _TR | _M | _BL | _B,
	['3'] = _T | _R | _M | _B,
	['4'] = _R | _TL | _M,
	['5'] = _T | _M | _B | _TL | _BR,
	['6'] = _T | _L | _M | _B | _BR,
	['7'] = _T | _R,
	['8'] = _T | _L | _R | _B | _M,
	['9'] = _T | _TL | _R | _B | _M,
	['<'] = _TRX | _BRX,
	['='] = _LC | _RC | _B,
	['>'] = _TLX | _BLX,
	['A'] = _L | _R | _T | _M,
	['B'] = _T | _TC | _TR | _RC | _BC |_BR | _B,
	['C'] = _T |_L | _B,
	['D'] = _T | _TC | _TR | _BC |_BR | _B,
	['E'] = _T | _L | _M | _B,
	['F'] =	_T | _LC | _L,
	['G'] = _T | _L | _B | _BR | _RC,
	['H'] = _L | _M | _R,
	['I'] = _T | _TC | _BC | _B,
	['J'] = _BL | _B | _BR | _TR,
	['K'] = _L | _LC | _TRX | _BRX,
	['L'] = _L |_B,
	['M'] = _L | _TLX | _TRX | _R,
	['N'] = _L | _TLX | _BRX | _R,
	['O'] = _T | _L | _B | _R,
	['P'] = _T | _L | _M | _TR,
	['Q'] = _T | _L | _B | _R | _BRX,
	['R'] = _T | _L | _M | _TR | _BRX,
	['S'] = _T | _M | _B | _TL | _BR,
	['T'] = _T | _TC | _BC,
	['U'] = _L | _B | _R,
	['V'] = _TLX | _BRX | _R,
	['W'] = _L | _BLX | _BRX | _R,
	['X'] = _TLX | _TRX | _BLX | _BRX,
	['Y'] = _TL | _M | _R | _B,
	['Z'] = _T | _TRX | _BLX | _B,
	['\\'] = _TLX | _BRX,
	['^'] = _BLX | _BRX,
	['_'] = _B,
	['a'] = _L | _R | _T | _M,
	['b'] = _L | _M | _B | _BR,
	['c'] = _BL | _M | _B,
	['d'] = _R | _M | _B | _BL,
	['e'] = _L | _T | _B | _LC,
	['f'] = _T | _L | _LC,
	['g'] = _T | _TL | _R | _B | _M,
	['h'] = _L | _M | _BR,
	['i'] = _BC,
	['j'] = _R | _B | _BL,
	['k'] = _TC | _BC | _TRX | _BRX,
	['l'] = _R,
	['m'] = _BL | _M | _BC | _BR,
	['n'] = _BL | _M | _BR,
	['o'] = _BL | _M | _B | _BR,
	['p'] = _T | _L | _M | _TR,
	['q'] = _T | _L | _B | _R | _BRX,
	['r'] = _BC | _M,
	['s'] = _T | _M | _B | _TL | _BR,
	['t'] = _M | _BC | _TC,
	['u'] = _BL | _B | _BR,
	['v'] = _BLX | _BL,
	['w'] = _BL | _B | _BC | _BR,
	['x'] = _M | _BLX | _BRX,
	['y'] = _TLX | _TRX | _BC,
	['z'] =  _T | _TRX | _BLX | _B,
	['|'] = _TC | _BC,
	[255] = ~0,
};

ip_usbph_char ip_usbph_font_char(uint8_t c)
{
	return font_char[c];
}

static ip_usbph_digit font_digit[16] = {
	[0x0] = _T | _L | _R | _B,
	[0x1] = _R | _E,
	[0x2] = _T | _TR | _M | _BL | _B,
	[0x3] = _T | _R | _M | _B,
	[0x4] = _R | _TL | _M,
	[0x5] = _T | _M | _B | _TL | _BR,
	[0x6] = _T | _L | _M | _B | _BR,
	[0x7] = _T | _R,
	[0x8] = _T | _L | _R | _B | _M,
	[0x9] = _T | _TL | _R | _B | _M,
	[0xa] = _T | _R | _BL | _M | _B,
	[0xb] = _L | _M | _B | _BR,
	[0xc] = _BL | _M | _B,
	[0xd] = _R | _M | _B | _BL,
	[0xe] = _T | _L | _BR | _B | _M,
	[0xf] =	_T | _M | _L,
};

ip_usbph_digit ip_usbph_font_digit(uint8_t  c)
{
	if (!isxdigit(c)) {
		return 0;
	}

	if (c >= '0' && c <= '9') {
		c = c - '0';
	} else {
		c = toupper(c) - 'A';
	}

	return font_digit[c];
}


