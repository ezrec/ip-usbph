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

#include <ip-usbph-def.h>

struct ip_usbph *ip_usbph_acquire(int index);
void ip_usbph_release(struct ip_usbph *ph);

int ip_usbph_init(struct ip_usbph *ph);

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
 * Index 0 and 3 are 'forced ones'
 */
int ip_usbph_top_digit(struct ip_usbph *ph, int index, ip_usbph_digit digit);

int ip_usbph_top_char(struct ip_usbph *ph, int index, ip_usbph_char ch);

int ip_usbph_bot_char(struct ip_usbph *ph, int index, ip_usbph_char ch);

/*
 * Flush new characters to the display
 */
int ip_usbph_flush(struct ip_usbph *ph);

/*
 * Get the file descriptor to poll for
 * this phone device.
 */
int ip_usbph_key_fd(struct ip_usbph *ph);

/* Get keycode and is-up mask for a key.
 */
uint16_t ip_usbph_key_get(struct ip_usbph *ph);

#endif /* IP_USBPH_H */
