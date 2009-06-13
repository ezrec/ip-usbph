/*
 * Copyright 2006, Jason McMullan
 * Author: Jason McMullan <jason.mcmullan@gmail.com>
 *
 * Licensed under the LGPL v2
 */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <limits.h>

#include "argv.h"
#include "ip-usbph.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

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

struct {
	const char *name;
	ip_usbph_sym symbol;
} symbols[] = {
	{ .name = "Down", .symbol = IP_USBPH_SYMBOL_DOWN },
	{ .name = "Up", .symbol = IP_USBPH_SYMBOL_UP },
	{ .name = "Sat", .symbol = IP_USBPH_SYMBOL_SAT },
	{ .name = "Colon", .symbol = IP_USBPH_SYMBOL_COLON },
	{ .name = "Fri", .symbol = IP_USBPH_SYMBOL_FRI },
	{ .name = "Thu", .symbol = IP_USBPH_SYMBOL_THU },
	{ .name = "M_and_D", .symbol = IP_USBPH_SYMBOL_M_AND_D },
	{ .name = "Tue", .symbol = IP_USBPH_SYMBOL_TUE },
	{ .name = "Wed", .symbol = IP_USBPH_SYMBOL_WED },
	{ .name = "Mon", .symbol = IP_USBPH_SYMBOL_MON },
	{ .name = "Sun", .symbol = IP_USBPH_SYMBOL_SUN },
	{ .name = "Out", .symbol = IP_USBPH_SYMBOL_OUT },
	{ .name = "In", .symbol = IP_USBPH_SYMBOL_IN },
	{ .name = "New", .symbol = IP_USBPH_SYMBOL_NEW },
	{ .name = "Mute", .symbol = IP_USBPH_SYMBOL_MUTE },
	{ .name = "Lock", .symbol = IP_USBPH_SYMBOL_LOCK },
	{ .name = "Man", .symbol = IP_USBPH_SYMBOL_MAN },
	{ .name = "Balance", .symbol = IP_USBPH_SYMBOL_BALANCE },
	{ .name = "Decimal", .symbol = IP_USBPH_SYMBOL_DECIMAL },
};

static int cmd_backlight(struct ip_usbph *ph, int argc, char **argv)
{
	if (argc > 1) {
		return -EINVAL;
	}

	return ip_usbph_backlight(ph);
}

static int cmd_symbols(struct ip_usbph *ph, int argc, char **argv)
{
	int i;

	if (argc > 1) {
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(symbols); i++) {
		printf("%s\n", symbols[i].name);
	}

	return 0;
}

static int cmd_symbol(struct ip_usbph *ph, int argc, char **argv)
{
	int onoff = -EINVAL;
	int i;
	int err;

	if (argc != 3) {
		return -EINVAL;
	}

	if (strcasecmp(argv[2],"on") == 0) {
		onoff = 1;
	}

	if (strcasecmp(argv[2],"off") == 0) {
		onoff = 0;
	}

	if (onoff < 0) {
		return onoff;
	}

	for (i = 0; i < ARRAY_SIZE(symbols); i++) {
		if (strcasecmp(argv[1], symbols[i].name) == 0) {
			err = ip_usbph_symbol(ph, symbols[i].symbol, onoff);
			ip_usbph_flush(ph);
			return err;
		}
	}

	return -EINVAL;
}

static int cmd_digit(struct ip_usbph *ph, int argc, char **argv)
{
	int i;
	char *cp = argv[1];

	if (argc != 2) {
		return -EINVAL;
	}

	for (i = 0; cp[i] != 0; i++) {
		if (!isxdigit(cp[i]) && !isspace(cp[i])) {
			return -EINVAL;
		}
	}

	if (i > 11) {
		return -ENAMETOOLONG;
	}

	for (i = 0; cp[i] != 0; i++) {
		ip_usbph_top_digit(ph, i, ip_usbph_font_digit(cp[i]));
	}

	for (; i < 11; i++) {
		ip_usbph_top_digit(ph, i, 0);
	}
	
	return ip_usbph_flush(ph);
}

static int cmd_top(struct ip_usbph *ph, int argc, char **argv)
{
	int i;
	char *cp = argv[1];

	if (argc != 2) {
		return -EINVAL;
	}

	if (strlen(cp) > 8) {
		return -ENAMETOOLONG;
	}

	for (i = 0; cp[i] != 0; i++) {
		ip_usbph_top_char(ph, i, ip_usbph_font_char(cp[i]));
	}

	for (; i < 8; i++) {
		ip_usbph_top_char(ph, i, 0);
	}

	return ip_usbph_flush(ph);
}

static int cmd_bot(struct ip_usbph *ph, int argc, char **argv)
{
	int i;
	char *cp = argv[1];

	if (argc != 2) {
		return -EINVAL;
	}

	if (strlen(cp) > 4) {
		return -ENAMETOOLONG;
	}

	for (i = 0; cp[i] != 0; i++) {
		ip_usbph_bot_char(ph, i, ip_usbph_font_char(cp[i]));
	}

	for (; i < 4; i++) {
		ip_usbph_bot_char(ph, i, 0);
	}

	return ip_usbph_flush(ph);
}

static int cmd_keys(struct ip_usbph *ph, int argc, char **argv)
{
	int i;

	if (argc > 1) {
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(keymap); i++) {
		if (keymap[i] == NULL) {
			continue;
		}
		printf("%s\n", keymap[i]);
	}

	return 0;
}

static int cmd_key(struct ip_usbph *ph, int argc, char **argv)
{
	struct pollfd fds[1];
	int err, fd;
	int timeout = -1;

	if (argc > 2) {
		return -EINVAL;
	}

	if (argc == 2) {
		timeout = strtol(argv[1], NULL, 0);
	}

	fd = ip_usbph_key_fd(ph);
	if (fd < 0) {
		return fd;
	}

	fds[0].fd = fd;
	fds[0].events = POLLIN | POLLERR;
	fds[0].revents = 0;

	while ((err = poll(fds, 1, timeout)) == 1) {
		uint16_t key;

		if (fds[0].revents & POLLERR) {
			break;
		}

		key = ip_usbph_key_get(ph);
		if (key == IP_USBPH_KEY_INVALID) {
			break;
		}

		if ((key & IP_USBPH_KEY_PRESSED) != 0) {
			printf("%s\n", keymap[key & 0x1f]);
			fflush(stdout);
			return 0;
		}
	}

	printf("\n");
	fflush(stdout);
	return 0;
}

static int cmd_clear(struct ip_usbph *ph, int argc, char **argv)
{
	if (argc > 1) {
		return -EINVAL;
	}

	return ip_usbph_clear(ph);
}

struct {
	const char *name;
	const char *help;
	int (*cmd)(struct ip_usbph *ph, int argc, char **argv);
} cmds[] = {
	{ .name = "backlight", .help = "backlight                 Turn the backlight on for 7 seconds",
	  .cmd = cmd_backlight, },
	{ .name = "clear",     .help = "clear                     Clear the display",
	  .cmd = cmd_clear, },
	{ .name = "symbols",   .help = "symbols                   List all symbols",
	  .cmd = cmd_symbols, },
	{ .name = "symbol",    .help = "symbol <name> on|off      Turn on/off a symbol",
	  .cmd = cmd_symbol, },
	{ .name = "digit",     .help = "digit <digits>            Display digits on the digit line",
	  .cmd = cmd_digit, },
	{ .name = "top",       .help = "top <string>              Display characters on the top character line",
	  .cmd = cmd_top, },
	{ .name = "bot",       .help = "bot <string>              Display characters on the top character line",
	  .cmd = cmd_bot, },
	{ .name = "key",       .help = "key [timeout]             Wait for a keystroke, optional timeout in msec",
	  .cmd = cmd_key, },
	{ .name = "keys",      .help = "keys                      List all key names",
	  .cmd = cmd_keys },
};

static void usage(const char *prog)
{
	int i;

	fprintf(stderr, "Usage:\n"
			"%s <command>\n\n",
			prog);
	for (i = 0; i < ARRAY_SIZE(cmds); i++) {
		fprintf(stderr, "%s\n", cmds[i].help);
	}
	fprintf(stderr, "shell                     Shell mode\n");
	fprintf(stderr, "pipe                      Pipe mode\n");
}

static void rc_load(struct ip_usbph *ph)
{
	char rcfile[PATH_MAX];
	int fd;

	snprintf(rcfile, sizeof(rcfile), "%s/.ip-usbphrc", getenv("HOME"));
	fd = open(rcfile, O_RDONLY);
	if (fd >= 0) {
		ip_usbph_state_load(ph, fd);
		close(fd);
	}
}

static void rc_save(struct ip_usbph *ph)
{
	char rcfile[PATH_MAX];
	char rcfile_new[PATH_MAX];
	int fd;

	snprintf(rcfile, sizeof(rcfile_new), "%s/.ip-usbphrc", getenv("HOME"));
	snprintf(rcfile_new, sizeof(rcfile_new), "%s/.ip-usbphrc~", getenv("HOME"));
	rename(rcfile, rcfile_new);
	fd = open(rcfile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0) {
		rename(rcfile_new, rcfile);
	} else {
		ip_usbph_state_save(ph, fd);
		close(fd);
	}
}

static int command(struct ip_usbph **pph, int argc, char **argv)
{
	int i, err;
	struct ip_usbph *ph = *pph;
	int (*cmd)(struct ip_usbph *ph, int argc, char **argv) = NULL;

	for (i = 0; i < ARRAY_SIZE(cmds); i++) {
		if (strcmp(argv[0],cmds[i].name) == 0) {
			cmd = cmds[i].cmd;
			break;
		}
	}

	if (cmd == NULL) {
		usage(argv[0]);
		return 0;
	}

	if (ph == NULL) {
		ph = ip_usbph_acquire(0);
		if (ph == NULL) {
			fprintf(stderr, "Can't find the IP-USBPH device. Is it plugged in?\n");
			exit(EXIT_FAILURE);
		}
		*pph = ph;

		rc_load(ph);
	}

	err = cmd(ph, argc, argv);
	if (err < 0) {
		if (err == -ETIMEDOUT) {
			/* Do nothing */
		} else if (err == -EINVAL) {
			fprintf(stderr, "Invalid arguments.\n");
		} else if (err == -ENAMETOOLONG) {
			fprintf(stderr, "String too long for command.\n");
		} else {
			errno = -err;
			perror(argv[0]);
		}
	}

	return err;
}

int main(int argc, char **argv)
{
	struct ip_usbph *ph = NULL;
	int err;

	if (argc < 2) {
		usage(argv[0]);
	}

	if (argc == 2 && (strcmp(argv[1], "shell") == 0 || strcmp(argv[1], "pipe") == 0)) {
		int pipe_mode = (strcmp(argv[1], "pipe") == 0);

		for (;;) {
			char buff[256];
			char *s;

			if (! pipe_mode) {
				printf("ip-usbph> ");
			};
			buff[0] = 0;
			s = fgets(buff, sizeof(buff), stdin);
			if (s == NULL || buff[0] == 0) {
				return EXIT_SUCCESS;
			}

			err = argv_from(buff, &argc, &argv);
			if (err < 0) {
				return EXIT_FAILURE;
			}
	
			if (argc == 0 || strcmp(argv[0], "exit") == 0) {
				return EXIT_SUCCESS;
			}

			err = command(&ph, argc, argv);
			if (err < 0) {
				fprintf(stderr, "%s\n", strerror(-err));
			}
			argv_free(argc, argv);
		}
	} else {
		err = command(&ph, argc-1, &argv[1]);
	}

	if (ph != NULL) {
		rc_save(ph);
		ip_usbph_release(ph);
	}

	return (err == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


