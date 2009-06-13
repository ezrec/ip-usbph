/*
 * Copyright 2007, Jason McMullan
 * Author: Jason McMullan <jason.mcmullan@gmail.com>
 *
 * Licensed under the LGPL v2
 */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <usb.h>
#include <signal.h>

#include <sys/wait.h>

#include "ip-usbph.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

struct ip_usbph {
	usb_dev_handle *usb;
	int key_pipe[2];
	pid_t key_pid;
	unsigned code_mask;
	uint8_t code_set[7][8];
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

static const uint8_t code_set[7][8] = {
	{ 0x02, 0xC1, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0xC1, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0xC1, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0xC1, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0xC1, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0xC1, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x02, 0x61, 0x5E, 0x00, 0x00, 0x00, 0x00, 0x00, },
};

static pid_t key_monitor(struct ip_usbph *ph)
{
	pid_t pid;
	int len, err;

	err = pipe(ph->key_pipe);
	if (err < 0) {
		return (pid_t)-1;
	}

	pid = fork();
	if (pid < 0) {
		return pid;
	}

	if (pid != 0) {
		ph->key_pid = pid;
		return;
	}

	/* In forked task */
	for (;;) {
		int len;
		uint8_t report[8];
		
		len = usb_interrupt_read(ph->usb, 0x81, report, sizeof(report), 0);
		if (len < 0 || len != sizeof(report)) {
			if (len == -EAGAIN) {
				continue;
			}
			exit(EXIT_FAILURE);
		}

		if (report[0] != 0x02 ||
		    report[1] != 0x61 ||
		    report[2] != 0x90) {
		    	/* Skip over non-key reports */
		    	continue;
		}

		len = write(ph->key_pipe[1], &report[3], 1);
		if (len != 1) {
			fprintf(stderr,"ip-usbph: Key monitor got back %d writing to the pipe (%s).\n", len, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
}

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
					err = usb_clear_halt(usb, 3);
					err = usb_detach_kernel_driver_np(usb, 3);
					err = usb_claim_interface(usb, 3);
					if (err < 0) {
						usb_close(usb);
						return NULL;
					}
					ph = calloc(1, sizeof(*ph));
					assert(ph != NULL);
					ph->usb = usb;
					memcpy(ph->code_set, code_set, sizeof(code_set));
					err = key_monitor(ph);
					if (err < 0) {
						usb_close(ph->usb);
						free(ph);
						return NULL;
					}
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

	kill(ph->key_pid, SIGTERM);
	usb_close(ph->usb);
	free(ph);
}

static int ip_usbph_raw(struct ip_usbph *ph, const uint8_t cmd[8])
{
	int err;
	err = usb_control_msg(ph->usb, 
	                      USB_TYPE_CLASS | USB_RECIP_INTERFACE,
	                      USB_REQ_SET_CONFIGURATION,
	                      0x202,
	                      0x03,
	                      (uint8_t *)cmd, 8, 0);

	return (err < 0) ? err : 0;
}

int ip_usbph_init(struct ip_usbph *ph)
{
	const uint8_t init[8] = { 0x02, 0x00, 0x00, 0x00,
		               0x00, 0x00, 0x00, 0x00 };
	return ip_usbph_raw(ph, init);
}

/* Save state to a fd
 *
 * Returns length written
 */
int ip_usbph_state_save(struct ip_usbph *ph, int fd)
{
	FILE *ouf = fdopen(dup(fd), "w");
	int i, j;

	if (ouf == NULL) {
		return -ENXIO;
	}

	for (i = 0; i < ARRAY_SIZE(ph->code_set); i++) {
		for (j = 0; j < ARRAY_SIZE(ph->code_set[i]); j++) {
			fprintf(ouf, "0x%.2x%c", ph->code_set[i][j], (j == (ARRAY_SIZE(ph->code_set[i])-1)) ? '\n' : ' ');
		}
	}

	fclose(ouf);
	return 5 * ARRAY_SIZE(ph->code_set) * ARRAY_SIZE(ph->code_set[0]);
}

/* Restore state from a fd
 */
int ip_usbph_state_load(struct ip_usbph *ph, int fd)
{
	FILE *ouf = fdopen(dup(fd), "r");
	int i, j;

	if (ouf == NULL) {
		return -ENXIO;
	}

	for (i = 0; i < ARRAY_SIZE(ph->code_set); i++) {
		for (j = 0; j < ARRAY_SIZE(ph->code_set[i]); j++) {
			int count;
			count = fscanf(ouf, " 0x%hhx", &ph->code_set[i][j]);
			if (count != 1) {
				fclose(ouf);
				return -EINVAL;
			}
		}
	}

	fclose(ouf);
	return 5 * ARRAY_SIZE(ph->code_set) * ARRAY_SIZE(ph->code_set[0]);
}


int ip_usbph_backlight(struct ip_usbph *ph)
{
	const uint8_t backlight_on_7_sec[8] = { 0x02, 0x64, 0x12, 0x01, 0xFD, 0x00, 0x00, 0x00 };
	return ip_usbph_raw(ph, backlight_on_7_sec);
}

int ip_usbph_clear(struct ip_usbph *ph)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ph->code_set); i++) {
		memset(&ph->code_set[i][3], 0, 5);
		ip_usbph_raw(ph, &ph->code_set[i][0]);
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

static inline int code_bit(struct ip_usbph *ph, code_id code, int bit, int is_on)
{
	uint8_t *cmd;

	if (code == 0 || code >= CODE_MAX) {
		return -EINVAL;
	}

	cmd = &ph->code_set[code-1][0];

	if (is_on) {
		cmd[3 + (bit/8)] |= 1 << (bit % 8);
	} else {
		cmd[3 + (bit/8)] &= ~(1 << (bit % 8));
	}

	ph->code_mask |= (1 << (code-1));

	return 0;
}

int ip_usbph_flush(struct ip_usbph *ph)
{
	int i;
	int err = 0;

	for (i = 0; i < CODE_MAX && err == 0; i++) {
		if (ph->code_mask & (1 << i)) {
			err = ip_usbph_raw(ph, &ph->code_set[i][0]);
		}
	}

	ph->code_mask = 0;

	return err;
}

int ip_usbph_symbol(struct ip_usbph *ph, ip_usbph_sym sym, int is_on)
{
	return code_bit(ph, font_symbol[sym].code, font_symbol[sym].bit, is_on);
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
	int i;

	if (index != 0 && index != 3) {
		for (i = 0; i < 7; i++) {
			code_bit(ph,
				 xref_digit_segment[index][i].code,
			         xref_digit_segment[index][i].bit,
			         digit & (1 << i));
		}
	} else if (index >= 0 && index < 11) {
		i = 7;

		code_bit(ph,
			 xref_digit_segment[index][i].code,
		         xref_digit_segment[index][i].bit,
		         digit & (1 << i));
	} else {
		return -EINVAL;
	}

	return 0;
}

/*
 * SEG0 - B  / BC  / TC / T
 * SEG1 - BR / BRX / RC / TRX / TR
 * SEG2 - BL / BLX / LC / TLX / TL
 */
static const struct {
	code_id code;
	int bit;
} top_char_seg[][3] = {
	[0] = {
		{ .code = CODE_C1_54, .bit = 17 },
		{ .code = CODE_C1_54, .bit = 8 },
		{ .code = CODE_C1_54, .bit = 24 },
	},
	[1] = {
		{ .code = CODE_C1_4F, .bit = 33 },
		{ .code = CODE_C1_4F, .bit = 24 },
		{ .code = CODE_C1_54, .bit =  0 },
	},
	[2] = {
		{ .code = CODE_C1_4F, .bit =  9 },
		{ .code = CODE_C1_4F, .bit =  0 },
		{ .code = CODE_C1_4F, .bit = 16 },
	},
	[3] = {
		{ .code = CODE_C1_40, .bit =  9 },
		{ .code = CODE_C1_40, .bit = 16 },
		{ .code = CODE_C1_40, .bit =  0 },
	},
	[4] = {
		{ .code = CODE_C1_40, .bit = 33 },
		{ .code = CODE_C1_4A, .bit = 32 },
		{ .code = CODE_C1_40, .bit = 24 },
	},
	[5] = {
		{ .code = CODE_C1_4A, .bit = 25 },
		{ .code = CODE_C1_4A, .bit = 16 },
		{ .code = CODE_C1_45, .bit =  0 },
	},
	[6] = {
		{ .code = CODE_C1_4A, .bit =  9 },
		{ .code = CODE_C1_4A, .bit =  0 },
		{ .code = CODE_C1_45, .bit =  8 },
	},
	[7] = {
		{ .code = CODE_C1_45, .bit = 25 },
		{ .code = CODE_C1_45, .bit = 16 },
		{ .code = CODE_C1_45, .bit = 32 },
	},
};

/*
 * SEG0 - B  / BC  / TC / T
 * SEG1 - BR / BRX / RC / TRX / TR
 * SEG2 - BL / BLX / LC / TLX / TL
 */
static const struct {
	ip_usbph_char mask;
	int seg;
	int bit;
} top_seg_map[14] = {
	{ .mask = IP_USBPH_SEG_B,   .seg = 0, .bit = 0 },
	{ .mask = IP_USBPH_SEG_BC,  .seg = 0, .bit = 1 },
	{ .mask = IP_USBPH_SEG_TC,  .seg = 0, .bit = 2 },
	{ .mask = IP_USBPH_SEG_T,   .seg = 0, .bit = 3 },
	{ .mask = IP_USBPH_SEG_BR,  .seg = 1, .bit = 0 },
	{ .mask = IP_USBPH_SEG_BRX, .seg = 1, .bit = 1 },
	{ .mask = IP_USBPH_SEG_RC,  .seg = 1, .bit = 2 },
	{ .mask = IP_USBPH_SEG_TRX, .seg = 1, .bit = 3 },
	{ .mask = IP_USBPH_SEG_TR,  .seg = 1, .bit = 4 },
	{ .mask = IP_USBPH_SEG_BL,  .seg = 2, .bit = 0 },
	{ .mask = IP_USBPH_SEG_BLX, .seg = 2, .bit = 1 },
	{ .mask = IP_USBPH_SEG_LC,  .seg = 2, .bit = 2 },
	{ .mask = IP_USBPH_SEG_TLX, .seg = 2, .bit = 3 },
	{ .mask = IP_USBPH_SEG_TL,  .seg = 2, .bit = 4 },
};

int ip_usbph_top_char(struct ip_usbph *ph, int index, ip_usbph_char ch)
{
	int i;

	if (ch & IP_USBPH_SEG_M) {
		ch |= IP_USBPH_SEG_RC | IP_USBPH_SEG_LC;
	}

	for (i = 0; i < 14; i++) {
		code_bit(ph,
			 top_char_seg[index][top_seg_map[i].seg].code,
			 top_char_seg[index][top_seg_map[i].seg].bit +
			   top_seg_map[i].bit, top_seg_map[i].mask & ch);
	}

	return 0;
}

/*
 * SEG0 - T  / TL  / TLX / LC / BC  / BLX / BL
 * SEG1 - TR / TRX / TC  / RC / BRC / BR  / B
 */
static const struct {
	ip_usbph_char mask;
	int seg;
	int bit;
} bot_seg_map[14] = {
	{ .mask = IP_USBPH_SEG_T,   .seg = 0, .bit = 0 },
	{ .mask = IP_USBPH_SEG_TL,  .seg = 0, .bit = 1 },
	{ .mask = IP_USBPH_SEG_TLX, .seg = 0, .bit = 2 },
	{ .mask = IP_USBPH_SEG_LC,  .seg = 0, .bit = 3 },
	{ .mask = IP_USBPH_SEG_BC,  .seg = 0, .bit = 4 },
	{ .mask = IP_USBPH_SEG_BLX, .seg = 0, .bit = 5 },
	{ .mask = IP_USBPH_SEG_BL,  .seg = 0, .bit = 6 },
	{ .mask = IP_USBPH_SEG_TR,  .seg = 1, .bit = 0 },
	{ .mask = IP_USBPH_SEG_TRX, .seg = 1, .bit = 1 },
	{ .mask = IP_USBPH_SEG_TC,  .seg = 1, .bit = 2 },
	{ .mask = IP_USBPH_SEG_RC,  .seg = 1, .bit = 3 },
	{ .mask = IP_USBPH_SEG_BRX, .seg = 1, .bit = 4 },
	{ .mask = IP_USBPH_SEG_BR,  .seg = 1, .bit = 5 },
	{ .mask = IP_USBPH_SEG_B,   .seg = 1, .bit = 6 },
};

static const struct {
	code_id code;
	int bit;
} bot_char_seg[][2] = {
	[0] = {
		{ .code = CODE_C1_54, .bit = 32 },
		{ .code = CODE_C1_59, .bit =  0 },
	},
	[1] = {
		{ .code = CODE_C1_59, .bit =  8 },
		{ .code = CODE_C1_59, .bit = 16 },
	},
	[2] = {
		{ .code = CODE_C1_59, .bit = 24 },
		{ .code = CODE_C1_59, .bit = 32 },
	},
	[3] = {
		{ .code = CODE_61_5E, .bit =  0 },
		{ .code = CODE_61_5E, .bit =  8 },
	},
};

int ip_usbph_bot_char(struct ip_usbph *ph, int index, ip_usbph_char ch)
{
	int i;

	if (ch & IP_USBPH_SEG_M) {
		ch |= IP_USBPH_SEG_RC | IP_USBPH_SEG_LC;
	}

	for (i = 0; i < 14; i++) {
		code_bit(ph,
			 bot_char_seg[index][bot_seg_map[i].seg].code,
			 bot_char_seg[index][bot_seg_map[i].seg].bit +
			   bot_seg_map[i].bit, bot_seg_map[i].mask & ch);
	}

	return 0;
}

int ip_usbph_key_fd(struct ip_usbph *ph)
{
	return ph->key_pipe[0];
}

uint8_t ip_usbph_key_get(struct ip_usbph *ph)
{
	uint8_t code;
	uint8_t key;
	int status;
	int err;

	if (waitpid(ph->key_pid, &status, WNOHANG) == ph->key_pid) {
		if (WIFEXITED(status) || WIFSIGNALED(status)) {
			fprintf(stderr, "ip-usbph: Key monitor killed.\n");
			exit(EXIT_FAILURE);
		}
	}

	err = read(ph->key_pipe[0], &code, 1);
	if (err != 1) {
		return IP_USBPH_KEY_INVALID;
	}

	return key;
}
	
