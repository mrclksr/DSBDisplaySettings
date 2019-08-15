/*-
 * Copyright (c) 2018 Marcel Kaiser. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <math.h>
#include <err.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/dpms.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include "libdsbds.h"
#include "dsbcfg/dsbcfg.h"

#define LCD_LEVELS_OID		"hw.acpi.video.lcd%d.levels"
#define LCD_BRIGHTNESS_OID	"hw.acpi.video.lcd%d.brightness"
#define CMD_XRANDR_MODE		PATH_XRANDR " --output %s --mode %s --rate %f"
#define CMD_XRANDR_GAMMA	PATH_XRANDR " --output %s --gamma %f:%f:%f"
#define CMD_XRANDR_BRIGHTNESS	PATH_XRANDR " --output %s --brightness %f"
#define CMD_XRANDR_ON		PATH_XRANDR " --output %s --auto"
#define CMD_XRANDR_OFF		PATH_XRANDR " --output %s --off"
#define CMD_XRANDR_INFO		PATH_XRANDR " --verbose"

#define CHECKNULL(expr) do {				\
	if ((expr) == NULL)				\
		err(1, "Uexpected command output");	\
} while(0)

typedef struct lvds_info_s {
	int	    unit;
	int	    levels[100];
	size_t	    nlevels;
} lvds_info;

typedef struct dsbds_mode_s {
	char	     mode[12];
	double	     rate;
} dsbds_mode;

typedef struct dsbds_output_s {
	int	     id;
	int	     curmode;
	int	     preferred;
	bool	     connected;
	bool	     is_lvds;
	char	     name[12];
	size_t	     nmodes;
	double	     brightness;
	double	     red;
	double	     green;
	double	     blue;
	lvds_info    lvds;
	dsbds_mode   modes[24];
} dsbds_output;

struct dsbds_scr_s {
	int	     screen;
	size_t	     noutputs;
	Display	     *display;
	dsbds_output outputs[64];
};

static int init_lvds_info(int, lvds_info *);

int
dsbds_update_screen(dsbds_scr *scr)
{
	char   ln[1024], *p, *q;
	bool   found_screen;
	FILE   *fp;
	size_t lc, no, mc;

	(void)setlocale(LC_NUMERIC, "C");
	if ((fp = popen(CMD_XRANDR_INFO, "r")) == NULL)
		err(1, "popen()");
	found_screen = false;
	for (lc = 0, no = -1; fgets(ln, sizeof(ln), fp) != NULL; lc++) {
		if (strncmp(ln, "Screen", 6) == 0) {
			if (found_screen)
				break;
			CHECKNULL(p = strtok(ln + 7, ":"));
			if (strtol(p, NULL, 10) == scr->screen)
				found_screen = true;
			continue;
		} else if (!found_screen)
			continue;
		if (!isspace(ln[0])) {
			no++;
			if (no + 1 >= sizeof(scr->outputs)) {
				warnx("Max. number of outputs exceeded");
				return (-1);
			}
			scr->noutputs = no + 1;
			scr->outputs[no].nmodes = 0;
			scr->outputs[no].curmode   = -1;
			scr->outputs[no].preferred = -1;
			CHECKNULL(p = strtok(ln, " "));
			(void)strncpy(scr->outputs[no].name, p,
			    sizeof(scr->outputs[no].name) - 1);
			CHECKNULL(p = strtok(NULL, " "));
			if (strcmp(p, "disconnected") == 0)
				scr->outputs[no].connected = false;
			else
				scr->outputs[no].connected = true;
		} else if (ln[0] == '\t') {
			CHECKNULL(p = strtok(ln + 1, " "));
			if ((q = strtok(NULL, " ")) == NULL)
				continue;
			if (strcmp(p, "Identifier:") == 0) {
				CHECKNULL(p = strtok(q, ":"));
				scr->outputs[no].id = strtol(p, NULL, 16);
			} else if (strcmp(p, "Gamma:") == 0) {
				CHECKNULL(p = strtok(q, ":"));
				scr->outputs[no].red = strtod(p, NULL);
				CHECKNULL(p = strtok(NULL, ":"));
				scr->outputs[no].green = strtod(p, NULL);
				CHECKNULL(p = strtok(NULL, ":"));
				scr->outputs[no].blue = strtod(p, NULL);
			} else if (strcmp(p, "Brightness:") == 0) {
				scr->outputs[no].brightness = strtod(q, NULL);
			}
		} else if (strncmp(ln, "        v:", 10) == 0) {
			p = ln + strlen(ln);
			while (p != ln && *p != ' ') {
				if (isalpha(*p))
					*p = '\0';
				p--;
			}
			mc = scr->outputs[no].nmodes;
			if (mc >= sizeof(scr->outputs[no].modes) /
			    sizeof(scr->outputs[no].modes[0])) {
				warnx("Max. number of modes exceeded");
				continue;
			}
			scr->outputs[no].modes[mc].rate = strtod(++p, NULL);
			scr->outputs[no].nmodes++;
		} else if (strncmp(ln, "        h:", 10) == 0) {
			continue;
		} else if (strncmp(ln, "  ", 2) == 0) {
			for (p = ln + 2; isspace(*p); p++)
				;
			for (q = p; *q != '\0' && !isspace(*q); q++)
				;
			if (*q == '\0') {
				warn("Unexpected command output: %s", ln);
				continue;
			}	
			*q++ = '\0';
			mc = scr->outputs[no].nmodes;
			if (mc >= sizeof(scr->outputs[no].modes) /
			    sizeof(scr->outputs[no].modes[0])) {
				warnx("Max. number of modes exceeded");
				continue;
			}
			(void)strncpy(scr->outputs[no].modes[mc].mode, p,
			    sizeof(scr->outputs[no].modes[mc].mode) - 1);
			if (strstr(q, "*current") != NULL)
				scr->outputs[no].curmode = mc;
			if (strstr(q, "preferred") != NULL)
				scr->outputs[no].preferred = mc;
		}
	}
	(void)fclose(fp);
	if (!found_screen) {
		warnx("Couldn't find screen %d", scr->screen);
		return (-1);
	}
	return (0);
}

dsbds_scr *
dsbds_init_screen(Display *display)
{
	size_t	  i, j;
	dsbds_scr *scr;

	if ((scr = malloc(sizeof(dsbds_scr))) == NULL)
		return (NULL);
	(void)memset(scr, 0, sizeof(dsbds_scr));
	scr->screen  = XDefaultScreen(display);
	scr->display = display;
	if (dsbds_update_screen(scr) != 0) {
		free(scr);
		return (NULL);
	}
	for (i = j = 0; i < scr->noutputs; i++) {
		if (strncmp(scr->outputs[i].name, "LVDS", 4) != 0)
			continue;
		(void)init_lvds_info(j++, &scr->outputs[i].lvds);
	}
	return (scr);
}

int
dsbds_set_mode(dsbds_scr *scr, int output, int mode)
{
	char cmd[sizeof(CMD_XRANDR_MODE) + 42];
	
	if (output < 0 || (size_t)output > scr->noutputs)
		return (-1);
	if (mode < 0) {
		if (dsbds_set_off(scr, output) == 0)
			return (0);
		return (-1);
	} else if ((size_t)mode > scr->outputs[output].nmodes)
		return (-1);
	(void)setlocale(LC_NUMERIC, "C");
	(void)snprintf(cmd, sizeof(cmd) - 1,
	    CMD_XRANDR_MODE,
	    scr->outputs[output].name,
	    scr->outputs[output].modes[mode].mode,
	    scr->outputs[output].modes[mode].rate);
	if (system(cmd) == 0) {
		dsbds_update_screen(scr);
		return (0);
	}
	return (-1);
}

void
dsbds_set_brightness(dsbds_scr *scr, int output, double brightness)
{
	char cmd[sizeof(CMD_XRANDR_BRIGHTNESS) + 33];
	
	if (output < 0 || (size_t)output > scr->noutputs ||
	    brightness < 0 || brightness > 1)
		return;
	(void)setlocale(LC_NUMERIC, "C");
	(void)snprintf(cmd, sizeof(cmd) - 1,
	    CMD_XRANDR_BRIGHTNESS,
	    scr->outputs[output].name, brightness);
	if (system(cmd) == 0)
		scr->outputs[output].brightness = brightness;
}

inline int
dsbds_mode_count(dsbds_scr *scr, int output)
{
	if (scr == NULL || output < 0 || (size_t)output > scr->noutputs)
		return (-1);
	return (scr->outputs[output].nmodes);
}

inline int
dsbds_output_count(dsbds_scr *scr)
{
	if (scr == NULL)
		return (-1);
	return (scr->noutputs);
}

const char *
dsbds_output_name(dsbds_scr *scr, int output)
{
	if (scr == NULL || output < 0 || (size_t)output > scr->noutputs)
		return (NULL);
	return (scr->outputs[output].name);
}

bool
dsbds_connected(dsbds_scr *scr, int output)
{
	if (scr == NULL || output < 0 || (size_t)output > scr->noutputs)
		return (false);
	return (scr->outputs[output].connected);
}

bool
dsbds_enabled(dsbds_scr *scr, int output)
{
	if (scr == NULL || output < 0 || (size_t)output > scr->noutputs)
		return (false);
	return (scr->outputs[output].curmode == -1 ? false : true);
}
	
bool
dsbds_is_lvds(dsbds_scr *scr, int output)
{
	if (scr == NULL || output < 0 || (size_t)output > scr->noutputs)
		return (false);
	return (scr->outputs[output].is_lvds);
}

double
dsbds_get_rate(dsbds_scr *scr, int output, int modeno)
{
	if (!dsbds_connected(scr, output))
		return (0);
	if (modeno < 0 || (size_t)modeno > scr->outputs[output].nmodes)
		return (0);
	return (scr->outputs[output].modes[modeno].rate);
}

int
dsbds_get_modeinfo(dsbds_scr *scr, int output, int modeno,
	char ** const mode,  double *rate)
{
	if (!dsbds_connected(scr, output) || modeno < 0 ||
	    (size_t)modeno > scr->outputs[output].nmodes)
		return (-1);
	*mode = scr->outputs[output].modes[modeno].mode;
	*rate = scr->outputs[output].modes[modeno].rate;

	return (0);
}

int
dsbds_get_mode(dsbds_scr *scr, int output)
{
	if (!dsbds_connected(scr, output))
		return (-1);
	return (scr->outputs[output].curmode);
}

double
dsbds_get_brightness(dsbds_scr *scr, int output)
{
	return (scr->outputs[output].brightness);
}

void
dsbds_get_gamma(dsbds_scr *scr, int output, double *r, double *g, double *b)
{
	*r = scr->outputs[output].red;
	*g = scr->outputs[output].green;
	*b = scr->outputs[output].blue;
}

int
dsbds_set_gamma_chan(dsbds_scr *scr, int output, int chan, double val)
{
	char   cmd[sizeof(CMD_XRANDR_GAMMA) + 28];
	double r, g, b;

	if (!dsbds_enabled(scr, output))
		return (-1);
	r = scr->outputs[output].red;
	g = scr->outputs[output].green;
	b = scr->outputs[output].blue;

	if (chan == 0)
		r = val;
	else if (chan == 1)
		g = val;
	else
		b = val;
	(void)snprintf(cmd, sizeof(cmd) - 1, CMD_XRANDR_GAMMA,
	    scr->outputs[output].name, r, g, b);
	if (system(cmd) == 0) {
		scr->outputs[output].red   = r;
		scr->outputs[output].green = g;
		scr->outputs[output].blue  = b;
		return (0);
	}
	return (-1);
}

void
dsbds_set_blanktime(dsbds_scr *scr, int timeout)
{
	int unused, interval, blanking, exposures;

	XGetScreenSaver(scr->display, &unused, &interval, &blanking,
	    &exposures);
	XSetScreenSaver(scr->display, timeout * 60, interval, blanking,
	    exposures);
	(void)XFlush(scr->display);
}

int
dsbds_get_blanktime(dsbds_scr *scr)
{
	int timeout, interval, blanking, exposures;

	XGetScreenSaver(scr->display, &timeout, &interval, &blanking,
	    &exposures);
	return (timeout / 60);
}

void
dsbds_get_dpms_info(dsbds_scr *scr, bool *enabled, int *standby, int *suspend,
	int *off)
{
	BOOL   state;
	CARD16 level, standby16, suspend16, off16;

	if (!DPMSInfo(scr->display, &level, &state))
		*enabled = false;
	else
		*enabled = (bool)state;
	if (!DPMSGetTimeouts(scr->display, &standby16, &suspend16, &off16)) {
		*standby = 10; *suspend = *standby + 1; *off = *standby + 1;
	} else {
		*off	 = off16 / 60;
		*standby = standby16 / 60;
		*suspend = suspend16 / 60;
	}
}

void
dsbds_set_dpms_enabled(dsbds_scr *scr, bool enable)
{
	if (enable)
		(void)DPMSEnable(scr->display);
	else
		(void)DPMSDisable(scr->display);
	(void)XFlush(scr->display);
}

void
dsbds_set_dpms_timeouts(dsbds_scr *scr, int standby, int suspend, int off)
{
	/* Make sure that standby <= suspend <= off. */
	if (standby > suspend)
		suspend = standby;
	if (suspend > off)
		off = suspend;
	/* Set timeouts in seconds. */
	(void)DPMSSetTimeouts(scr->display, standby * 60, suspend * 60,
	    60 * off);
	(void)XFlush(scr->display);
}

int
dsbds_set_red(dsbds_scr *scr, int output, double val)
{
	return (dsbds_set_gamma_chan(scr, output, 0, val));
}

int
dsbds_set_green(dsbds_scr *scr, int output, double val)
{
	return (dsbds_set_gamma_chan(scr, output, 1, val));
}

int
dsbds_set_blue(dsbds_scr *scr, int output, double val)
{
	return (dsbds_set_gamma_chan(scr, output, 2, val));
}

int
dsbds_set_gamma(dsbds_scr *scr, int output, double gamma)
{
	char cmd[sizeof(CMD_XRANDR_GAMMA) + 28];

	if (!dsbds_enabled(scr, output))
		return (-1);
	(void)snprintf(cmd, sizeof(cmd) - 1, CMD_XRANDR_GAMMA,
	    scr->outputs[output].name, gamma, gamma, gamma);
	if (system(cmd) == 0) {
		scr->outputs[output].red   = gamma;
		scr->outputs[output].green = gamma;
		scr->outputs[output].blue  = gamma;
		return (0);
	}
	return (-1);	
}

int
dsbds_set_off(dsbds_scr *scr, int output)
{
	char cmd[sizeof(CMD_XRANDR_OFF) + 16];

	if (!dsbds_enabled(scr, output))
		return (0);
	(void)snprintf(cmd, sizeof(cmd) - 1, CMD_XRANDR_OFF,
	    scr->outputs[output].name);
	if (system(cmd) == 0) {
		dsbds_update_screen(scr);
		if (!dsbds_enabled(scr, output))
			return (0);
	}
	return (-1);
}

int
dsbds_set_on(dsbds_scr *scr, int output)
{
	char cmd[sizeof(CMD_XRANDR_ON) + 16];

	if (dsbds_enabled(scr, output))
		return (0);
	(void)snprintf(cmd, sizeof(cmd) - 1, CMD_XRANDR_ON,
	    scr->outputs[output].name);
	if (system(cmd) == 0) {
		dsbds_update_screen(scr);
		if (dsbds_enabled(scr, output))
			return (0);
	}
	return (-1);
}

int
dsbds_get_lcd_brightness_level(dsbds_scr *scr, int output)
{
	int    val;
	char   oid[sizeof(LCD_BRIGHTNESS_OID)];
	size_t i, sz;

	if (!dsbds_is_lvds(scr, output))
		return (-1);
	sz = sizeof(int);
	(void)snprintf(oid, sizeof(oid), LCD_BRIGHTNESS_OID,
	    scr->outputs[output].lvds.unit);
	if (sysctlbyname(oid, &val, &sz, NULL, 0) == -1)
		return (-1);
	for (i = 0; i < scr->outputs[output].lvds.nlevels; i++) {
		if (scr->outputs[output].lvds.levels[i] == val)
			return (i);
	}
	return (-1);
}

int
dsbds_set_lcd_brightness_level(dsbds_scr *scr, int output, int level)
{
	char oid[sizeof(LCD_BRIGHTNESS_OID)];

	if (!dsbds_is_lvds(scr, output))
		return (-1);
	(void)snprintf(oid, sizeof(oid), LCD_BRIGHTNESS_OID,
	    scr->outputs[output].lvds.unit);
	if (sysctlbyname(oid, NULL, NULL,
	    &scr->outputs[output].lvds.levels[level],
	    sizeof(int)) == -1)
		return (-1);
	return (0);
}

int
dsbds_lcd_brightness_count(dsbds_scr *scr, int output)
{
	if (!dsbds_is_lvds(scr, output))
		return (-1);
	return (scr->outputs[output].lvds.nlevels);
}

static int
init_lvds_info(int unit, lvds_info *lvds)
{
	int    buf[100], tmp;
	char   oid[sizeof(LCD_LEVELS_OID)];
	bool   sorted;
	size_t n, i;

	n = sizeof(buf);
	(void)snprintf(oid, sizeof(oid), LCD_LEVELS_OID, unit);
	if (sysctlbyname(oid, buf, &n, NULL, 0) == -1)
		return (-1);
	for (sorted = false; !sorted;) {
		sorted = true;
		for (i = 0; i < n / sizeof(int) - 1; i++) {
			if (buf[i] > buf[i + 1]) {
				sorted = false;
				tmp = buf[i]; buf[i] = buf[i + 1];
				buf[i + 1] = tmp;
			}
		}
	}
	lvds->unit    = unit;
	lvds->nlevels = n / sizeof(int);
	for (i = 0; i < lvds->nlevels; i++)
		lvds->levels[i] = buf[i];
	return (0);
}

char *
dsbds_exec_backend(const char *cmd, int *ret)
{
	int    bufsz, rd, len, pfd[2];
	char  *buf;
	pid_t  pid;

	errno = 0;
	if ((pid = pipe(pfd)) == -1)
		return (NULL);
	switch (vfork()) {
	case -1:
		return (NULL);
	case  0:
		(void)close(pfd[0]);
		(void)dup2(pfd[1], 1);
		(void)dup2(pfd[1], 2);
		execlp("/bin/sh", "/bin/sh", "-c", cmd, NULL);
		exit(-1);
	default:
		(void)close(pfd[1]);
	}
	bufsz = len = rd = 0; buf = NULL;
	do {
		len += rd;
		if (len >= bufsz - 1) {
			if ((buf = realloc(buf, bufsz + 64)) == NULL)
				return (NULL);
			bufsz += 64;
		}
	} while ((rd = read(pfd[0], buf + len, bufsz - len - 1)) > 0);
	buf[len] = '\0';
	(void)waitpid(pid, ret, WEXITED);
	*ret = WEXITSTATUS(*ret);
	if (*ret == -1)
		free(buf);
	return (*ret == -1 ? NULL : buf);
}

static char *
get_script_path(const char *suffix)
{
	char   *path, *dir;
	size_t len;

	if ((dir = dsbcfg_mkdir(NULL)) == NULL)
		return (NULL);
	len = strlen(dir) + sizeof(PATH_SCRIPT) + 1;
	if (suffix != NULL)
		len += strlen(suffix) + 1;
	if ((path = malloc(len)) == NULL)
		return (NULL);
	(void)snprintf(path, len, "%s/%s%s", dir, PATH_SCRIPT,
	    suffix != NULL ? suffix : "");
	return (path);
}

static FILE *
open_tempfile(char **path)
{
	int  fd;
	char *p;
	FILE *fp;

	if ((p = get_script_path(".XXXX")) == NULL)
		return (NULL);
	if ((fd = mkstemp(p)) == -1) {
		warn("mkstemp(%s)", p); free(p);
		return (NULL);
	}
	if ((fp = fdopen(fd, "w")) == NULL) {
		warn("fdopen()"); free(p); (void)close(fd);
		return (NULL);
	}
	*path = p;

	return (fp);
}

int
dsbds_save_settings(dsbds_scr *scr)
{
	int  i, ret, dpms[3];
	bool dpms_on;
	char *tmpath, *topath;
	FILE *fp;

	(void)setlocale(LC_NUMERIC, "C");
	if ((fp = open_tempfile(&tmpath)) == NULL)
		return (-1);
	if ((topath = get_script_path(NULL)) == NULL)
		return (-1);
	dsbds_get_dpms_info(scr, &dpms_on, &dpms[0], &dpms[1], &dpms[2]);
	(void)fprintf(fp, "#!/bin/sh\n");
	(void)fprintf(fp, "%s -B %d -d %d:%d:%d:%d\n",
	    PATH_BACKEND,
	    dsbds_get_blanktime(scr), !!dpms_on, dpms[0], dpms[1], dpms[2]);
	for (i = 0; i < dsbds_output_count(scr); i++) {
		if (!dsbds_connected(scr, i))
			continue;
		if (dsbds_is_lvds(scr, i)) {
			ret = fprintf(fp,
			    "%s -b %f -m %d -l %d -g %f:%f:%f %s\n",
			    PATH_BACKEND,
			    dsbds_get_brightness(scr, i),
			    dsbds_enabled(scr, i) ? dsbds_get_mode(scr, i) : -1,
			    dsbds_get_lcd_brightness_level(scr, i),
			    scr->outputs[i].red, scr->outputs[i].green,
			    scr->outputs[i].blue,
			    dsbds_output_name(scr, i));
		} else {
			ret = fprintf(fp,
			    "%s -b %f -m %d -g %f:%f:%f %s\n",
			    PATH_BACKEND,
			    dsbds_get_brightness(scr, i),
			    dsbds_enabled(scr, i) ? dsbds_get_mode(scr, i) : -1,
			    scr->outputs[i].red, scr->outputs[i].green,
			    scr->outputs[i].blue,
			    dsbds_output_name(scr, i));
		}
		if (ret < 0) {
			warn("Writing on %s failed", tmpath);
			free(tmpath); free(topath);
			return (-1);
		}
	}
	(void)fclose(fp);

	if ((ret = rename(tmpath, topath)) == -1)
		warn("rename(%s, %s)", tmpath, topath);
	if (chmod(topath, S_IRWXU) == -1)
		warn("chmod(%s)", topath);
	free(tmpath); free(topath);

	return (ret);
}
