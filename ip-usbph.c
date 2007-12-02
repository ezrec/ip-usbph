/*
 * $Id$
 *
 * Copyright 2007, Jason McMullan
 * Author: Jason McMullan <jason.mcmullan@gmail.com>
 *
 */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <usb.h>

#include "ip-usbph.h"

struct ip_usbph {
	usb_dev_handle *usb;
};

typedef enum {
	CODE_C1_40 = 1,
	CODE_C1_45,
	CODE_C1_4A,
	CODE_C1_4F,
	CODE_C1_54,
	CODE_C1_59,
	CODE_61_5E,
	CODE_MAX,
} code_id;

static uint8_t code_set[7][8] = {
	{ 0x02, 0xC1, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0xC1, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0xC1, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0xC1, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0xC1, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0xC1, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0x61, 0x5E, 0x00, 0x00, 0x00, 0x00, 0x00, },
};

struct ip_usbph *ip_usbph_acquire(int index)
{
	struct usb_bus *busses, *bus;
	struct ip_usbph *ph;
	int err;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();

	for (bus = busses; bus != NULL; bus = bus->next) {
		struct usb_device *dev;
		struct usb_dev_handle *usb;

		for (dev = bus->devices; dev != NULL; dev = dev->next) {
			if (dev->descriptor.idVendor == 0x04d9 && 
				dev->descriptor.idProduct == 0x0602 &&
				dev->descriptor.iManufacturer == 1 &&
				dev->descriptor.iProduct == 2 &&
				dev->descriptor.iSerialNumber == 0 &&
				dev->descriptor.bNumConfigurations == 1) {

				usb = usb_open(dev);
				if (usb == NULL) {
					fprintf(stderr, "Can't open IP-USBPH device: %s\n", usb_strerror());
					continue;
				}
				if (index <= 0) {
					err = usb_reset(usb);
					err = usb_set_configuration(usb, 1);
					err = usb_detach_kernel_driver_np(usb, 3);
					err = usb_claim_interface(usb, 3);
					if (err < 0) {
						usb_close(usb);
						return NULL;
					}
					ph = calloc(1, sizeof(*ph));
					assert(ph != NULL);
					ph->usb = usb;
					return ph;
				}
				index--;
			}
		}
	}

	return NULL;
}

void ip_usbph_release(struct ip_usbph *ph)
{
	assert(ph != NULL);
	assert(ph->usb != NULL);

	usb_reset(ph->usb);
	usb_close(ph->usb);
	free(ph);
}

int ip_usbph_raw(struct ip_usbph *ph, const uint8_t cmd[8])
{
	int err;
	err = usb_control_msg(ph->usb, 
	                      USB_TYPE_CLASS | USB_RECIP_INTERFACE,
	                      USB_REQ_SET_CONFIGURATION,
	                      0x202,
	                      0x03,
	                      cmd, 8, 0);

	return (err < 0) ? err : 0;
}

int ip_usbph_init(struct ip_usbph *ph)
{
	const uint8_t init[8] = { 0x02, 0x00, 0x00, 0x00,
		               0x00, 0x00, 0x00, 0x00 };
	return ip_usbph_raw(ph, init);
}

int ip_usbph_backlight(struct ip_usbph *ph)
{
	const uint8_t backlight_on_7_sec[8] = { 0x02, 0x64, 0x12, 0x01, 0xFD, 0x00, 0x00, 0x00 };
	return ip_usbph_raw(ph, backlight_on_7_sec);
}

int ip_usbph_clear(struct ip_usbph *ph)
{
	int i;

	for (i = 0; i < 7; i++) {
		memset(&code_set[i][3], 0, 5);
		ip_usbph_raw(ph, &code_set[i][0]);
	}
}

static const struct {
	int code;
	int bit;
} font_symbol[] = {
	[IP_USBPH_SYMBOL_DOWN] = { .code = CODE_C1_40, .bit = 21 },
	[IP_USBPH_SYMBOL_UP] =   { .code = CODE_C1_40, .bit = 22 },
	[IP_USBPH_SYMBOL_SAT] =  { .code = CODE_C1_45, .bit =  7 },
	[IP_USBPH_SYMBOL_COLON] = {.code = CODE_C1_4A, .bit =  5 },
	[IP_USBPH_SYMBOL_FRI] =  { .code = CODE_C1_4A, .bit =  6 },
	[IP_USBPH_SYMBOL_THU] =  { .code = CODE_C1_4A, .bit =  7 },
	[IP_USBPH_SYMBOL_M_AND_D]={.code = CODE_C1_4F, .bit =  5 },
	[IP_USBPH_SYMBOL_TUE] =  { .code = CODE_C1_4F, .bit =  6 },
	[IP_USBPH_SYMBOL_WED] =  { .code = CODE_C1_4F, .bit =  7 },
	[IP_USBPH_SYMBOL_MON] =  { .code = CODE_C1_4F, .bit = 30 },
	[IP_USBPH_SYMBOL_SUN] =  { .code = CODE_C1_4F, .bit = 31 },
	[IP_USBPH_SYMBOL_OUT] =  { .code = CODE_C1_54, .bit = 29 },
	[IP_USBPH_SYMBOL_IN] =   { .code = CODE_C1_54, .bit = 30 },
	[IP_USBPH_SYMBOL_NEW] =  { .code = CODE_C1_54, .bit = 39 },
	[IP_USBPH_SYMBOL_MUTE] = { .code = CODE_C1_59, .bit =  7 },
	[IP_USBPH_SYMBOL_LOCK] = { .code = CODE_C1_59, .bit = 15 },
	[IP_USBPH_SYMBOL_MAN]  = { .code = CODE_C1_59, .bit = 23 },
	[IP_USBPH_SYMBOL_BALANCE]={.code = CODE_C1_59, .bit = 31 },
	[IP_USBPH_SYMBOL_DECIMAL]={.code = CODE_61_5E, .bit =  7 },
};

static unsigned code_mask;

static inline int code_bit(code_id code, int bit, int is_on)
{
	uint8_t *cmd;

	if (code == 0 || code >= CODE_MAX) {
		return -EINVAL;
	}

	cmd = &code_set[code-1][0];

	if (is_on) {
		cmd[3 + (bit/8)] |= 1 << (bit % 8);
	} else {
		cmd[3 + (bit/8)] &= ~(1 << (bit % 8));
	}

	code_mask |= (1 << (code-1));

	return 0;
}

int ip_usbph_flush(struct ip_usbph *ph)
{
	int i;
	int err = 0;

	for (i = 0; i < CODE_MAX && err == 0; i++) {
		if (code_mask & (1 << i)) {
			err = ip_usbph_raw(ph, &code_set[i][0]);
		}
	}

	code_mask = 0;

	return err;
}

int ip_usbph_symbol(struct ip_usbph *ph, ip_usbph_sym sym, int is_on)
{
	return code_bit(font_symbol[sym].code, font_symbol[sym].bit, is_on);
}

#define SEG_T	0
#define SEG_B	1
#define SEG_TL	2
#define SEG_TR	3
#define SEG_BL	4
#define SEG_BR	5
#define SEG_M	6
#define SEG_E	7

static const struct {
	code_id code;
	int bit;
} xref_digit_segment[11][8] = {
	[0] = {
		[SEG_E] = { .code = CODE_C1_54, .bit = 31 },
	},
	[1] = {
		[SEG_BR] = { .code = CODE_C1_54, .bit = 13 },
		[SEG_M] = { .code = CODE_C1_54, .bit = 14 },
		[SEG_TR] = { .code = CODE_C1_54, .bit = 15 },
		[SEG_T] = { .code = CODE_C1_54, .bit = 16 },
		[SEG_B] = { .code = CODE_C1_54, .bit = 21 },
		[SEG_BL] = { .code = CODE_C1_54, .bit = 22 },
		[SEG_TL] = { .code = CODE_C1_54, .bit = 23 },
	},
	[2] = {
		[SEG_T] = { .code = CODE_C1_4F, .bit = 32 },
		[SEG_B] = { .code = CODE_C1_4F, .bit = 37 },
		[SEG_BR] = { .code = CODE_C1_4F, .bit = 38 },
		[SEG_TR] = { .code = CODE_C1_4F, .bit = 39 },
		[SEG_BL] = { .code = CODE_C1_54, .bit = 5 },
		[SEG_M] = { .code = CODE_C1_54, .bit = 6 },
		[SEG_TL] = { .code = CODE_C1_54, .bit = 7 },
	},
	[3] = {
		[SEG_E] = { .code = CODE_C1_4F, .bit = 29 },
	},
	[4] = {
		[SEG_T] = { .code = CODE_C1_4F, .bit = 8 },
		[SEG_B] = { .code = CODE_C1_4F, .bit = 13 },
		[SEG_BR] = { .code = CODE_C1_4F, .bit = 14 },
		[SEG_TR] = { .code = CODE_C1_4F, .bit = 15 },
		[SEG_BL] = { .code = CODE_C1_4F, .bit = 21 },
		[SEG_M] = { .code = CODE_C1_4F, .bit = 22 },
		[SEG_TL] = { .code = CODE_C1_4F, .bit = 23 },
	},
	[5] = {
		[SEG_BL] = { .code = CODE_C1_40, .bit = 5 },
		[SEG_M] = { .code = CODE_C1_40, .bit = 6 },
		[SEG_TL] = { .code = CODE_C1_40, .bit = 7 },
		[SEG_T] = { .code = CODE_C1_40, .bit = 8 },
		[SEG_B] = { .code = CODE_C1_40, .bit = 13 },
		[SEG_BR] = { .code = CODE_C1_40, .bit = 14 },
		[SEG_TR] = { .code = CODE_C1_40, .bit = 15 },
	},
	[6] = {
		[SEG_BL] = { .code = CODE_C1_40, .bit = 29 },
		[SEG_M] = { .code = CODE_C1_40, .bit = 30 },
		[SEG_TL] = { .code = CODE_C1_40, .bit = 31 },
		[SEG_T] = { .code = CODE_C1_40, .bit = 32 },
		[SEG_B] = { .code = CODE_C1_40, .bit = 37 },
		[SEG_BR] = { .code = CODE_C1_40, .bit = 38 },
		[SEG_TR] = { .code = CODE_C1_40, .bit = 39 },
	},
	[7] = {
		[SEG_T] = { .code = CODE_C1_4A, .bit = 24 },
		[SEG_B] = { .code = CODE_C1_4A, .bit = 29 },
		[SEG_BR] = { .code = CODE_C1_4A, .bit = 30 },
		[SEG_TR] = { .code = CODE_C1_4A, .bit = 31 },
		[SEG_BL] = { .code = CODE_C1_4A, .bit = 37 },
		[SEG_M] = { .code = CODE_C1_4A, .bit = 38 },
		[SEG_TL] = { .code = CODE_C1_4A, .bit = 39 },
	},
	[8] = {
		[SEG_T] = { .code = CODE_C1_4A, .bit = 8 },
		[SEG_B] = { .code = CODE_C1_4A, .bit = 13 },
		[SEG_BR] = { .code = CODE_C1_4A, .bit = 14 },
		[SEG_TR] = { .code = CODE_C1_4A, .bit = 15 },
		[SEG_BL] = { .code = CODE_C1_4A, .bit = 21 },
		[SEG_M] = { .code = CODE_C1_4A, .bit = 22 },
		[SEG_TL] = { .code = CODE_C1_4A, .bit = 23 },
	},
	[9] = {
		[SEG_T] = { .code = CODE_C1_45, .bit = 24 },
		[SEG_B] = { .code = CODE_C1_45, .bit = 29 },
		[SEG_BR] = { .code = CODE_C1_45, .bit = 30 },
		[SEG_TR] = { .code = CODE_C1_45, .bit = 31 },
		[SEG_BL] = { .code = CODE_C1_45, .bit = 37 },
		[SEG_M] = { .code = CODE_C1_45, .bit = 38 },
		[SEG_TL] = { .code = CODE_C1_45, .bit = 39 },
	},
	[10] = {
		[SEG_T] = { .code = CODE_C1_45, .bit = 6 },
		[SEG_B] = { .code = CODE_C1_45, .bit = 13 },
		[SEG_BR] = { .code = CODE_C1_45, .bit = 14 },
		[SEG_TR] = { .code = CODE_C1_45, .bit = 15 },
		[SEG_BL] = { .code = CODE_C1_45, .bit = 21 },
		[SEG_M] = { .code = CODE_C1_45, .bit = 22 },
		[SEG_TL] = { .code = CODE_C1_45, .bit = 23 },
	},
};

int ip_usbph_top_digit(struct ip_usbph *ph, int index, ip_usbph_digit digit)
{
	int code_mask = 0;
	int i;

	if (index != 0 && index != 3) {
		for (i = 0; i < 7; i++) {
			code_bit(xref_digit_segment[index][i].code,
			         xref_digit_segment[index][i].bit,
			         digit & (1 << i));
		}
	} else if (index >= 0 && index < 11) {
		i = 7;

		code_bit(xref_digit_segment[index][i].code,
		         xref_digit_segment[index][i].bit,
		         digit & (1 << i));
	} else {
		return -EINVAL;
	}

	return 0;
}


