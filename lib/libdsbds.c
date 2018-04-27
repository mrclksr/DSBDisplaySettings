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
#include <stdio.h>
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

#define LCD_LEVELS_OID	   "hw.acpi.video.lcd%d.levels"
#define LCD_BRIGHTNESS_OID "hw.acpi.video.lcd%d.brightness"

typedef struct dsbds_lvds_info_s {
	int  unit;
	int  nlevels;
	int  *levels;
} dsbds_lvds_info;

typedef struct dsbds_output_s {
	int	      nmodes;
	double        brightness;
	double        red;
	double        green;
	double        blue;
	RROutput      id;
	XRRModeInfo   **modes;
	XRRCrtcInfo   *crtc_info;
	XRROutputInfo *output_info;
	dsbds_lvds_info *lvds_info;
} dsbds_output;

typedef struct dsbds_scr_s {
	int	 noutputs;
	Display *display;
	dsbds_output **outputs;
	XRRScreenResources *xrr_scr;
} dsbds_scr;

static int  update_output(dsbds_scr *, int);
static int  get_rgbb(dsbds_scr *, int);
static int *get_lcd_brightness_levels(int, size_t *);
static XRRModeInfo **get_modes(dsbds_scr *, int, int *);
static dsbds_lvds_info *init_lvds_info(int);

static const int maxval = (1 << 16) - 1;

int
dsbds_set_mode(dsbds_scr *scr, int output, int mode)
{
	Status ret;

	XGrabServer(scr->display);
	ret = XRRSetCrtcConfig(scr->display,
		scr->xrr_scr,
		scr->outputs[output]->output_info->crtc,
		scr->xrr_scr->timestamp,
		scr->outputs[output]->crtc_info->x, 
		scr->outputs[output]->crtc_info->y,
		scr->outputs[output]->modes[mode]->id,
		scr->outputs[output]->crtc_info->rotation,
		scr->outputs[output]->crtc_info->outputs,
		scr->outputs[output]->crtc_info->noutput);
	XUngrabServer(scr->display);
	(void)update_output(scr, output);

	return (ret != RRSetConfigSuccess ? -1 : 0);
}

inline int
dsbds_mode_count(dsbds_scr *scr, int output)
{
	if (scr == NULL || output < 0 || output > scr->noutputs)
		return (-1);
	return (scr->outputs[output]->nmodes);
}

inline int
dsbds_output_count(dsbds_scr *scr)
{
	if (scr == NULL)
		return (-1);
	return (scr->noutputs);
}

dsbds_scr *
dsbds_init_screen(Display *display)
{
	int	      i, j;
	dsbds_scr    *scr;
	dsbds_output *tmp;
	XRRScreenResources *sr;

	sr = XRRGetScreenResources(display, DefaultRootWindow(display));
	if (sr == NULL)
		return (NULL);
	if ((scr = malloc(sizeof(dsbds_scr))) == NULL)
		return (NULL);
	scr->outputs = malloc(sr->noutput * sizeof(dsbds_output **));
	if (scr->outputs == NULL) {
		free(scr);
		return (NULL);
	}
	scr->xrr_scr  = sr;
	scr->display  = display;
	scr->noutputs = sr->noutput;
	for (i = 0; i < sr->noutput; i++) {
		if ((scr->outputs[i] = malloc(sizeof(dsbds_output))) == NULL)
			goto error;
		scr->outputs[i]->id	     = sr->outputs[i];
		scr->outputs[i]->lvds_info   = NULL;
		scr->outputs[i]->crtc_info   = NULL;
		scr->outputs[i]->modes	     = NULL;
		scr->outputs[i]->nmodes	     = 0;
		scr->outputs[i]->output_info = XRRGetOutputInfo(display, sr,
		    sr->outputs[i]);
		scr->noutputs = sr->noutput;
		if (scr->outputs[i]->output_info->connection == RR_Connected) {
			scr->outputs[i]->modes = get_modes(scr, i,
			    &scr->outputs[i]->nmodes);
			scr->outputs[i]->crtc_info =
			    XRRGetCrtcInfo(display, sr,
				scr->outputs[i]->output_info->crtc);
			get_rgbb(scr, i);
		}
		/* Sort outputs */
		j = 0;
		while (j < i) {
			if (strcmp(scr->outputs[j]->output_info->name,
			    scr->outputs[j + 1]->output_info->name) > 0) {
				tmp = scr->outputs[j];
				scr->outputs[j] = scr->outputs[j + 1];
				scr->outputs[j + 1] = tmp;
				if (--j < 0)
					j = 0;
			} else
				j++;
		}
	}
	for (i = j = 0; i < scr->noutputs; i++) {
		if (strncmp(scr->outputs[i]->output_info->name, "LVDS", 4) != 0)
			continue;
		scr->outputs[i]->lvds_info = init_lvds_info(j++);
	}
	return (scr);
error:
	for (i = 0; i < sr->noutput; i++) {
		free(scr->outputs[i]);
		free(scr->outputs[i]->lvds_info);
	}
	free(scr);

	return (NULL);
}

const char *
dsbds_output_name(dsbds_scr *scr, int output)
{
	if (scr == NULL || output < 0 || output > scr->noutputs)
		return (NULL);
	return (scr->outputs[output]->output_info->name);
}

bool
dsbds_connected(dsbds_scr *scr, int output)
{
	if (scr == NULL || output < 0 || output > scr->noutputs)
		return (false);
	if (scr->outputs[output]->output_info->connection == RR_Connected)
		return (true);
	return (false);
}

bool
dsbds_is_lvds(dsbds_scr *scr, int output)
{
	if (scr == NULL || output < 0 || output > scr->noutputs)
		return (false);
	if (scr->outputs[output]->lvds_info == NULL)
		return (false);
	return (true);
}

double
dsbds_get_rate(dsbds_scr *scr, int output, int modeno)
{
	XRRModeInfo *m;

	if (!dsbds_connected(scr, output))
		return (0);
	if (modeno < 0 || modeno > scr->outputs[output]->nmodes)
		return (0);
	m = scr->outputs[output]->modes[modeno];

	return ((double)m->dotClock / (double)(m->hTotal * m->vTotal));
}

int
dsbds_get_modeinfo(dsbds_scr *scr, int output, int modeno,
	int *width, int *height, double *rate)
{
	XRRModeInfo *m;

	if (!dsbds_connected(scr, output) || modeno < 0 ||
	    modeno > scr->outputs[output]->nmodes)
		return (-1);
	m = scr->outputs[output]->modes[modeno];
	*width = m->width; *height = m->height;
	*rate  = (double)m->dotClock / (double)(m->hTotal * m->vTotal);

	return (0);
}

int
dsbds_get_mode(dsbds_scr *scr, int output)
{
	int i;

	for (i = 0; i < scr->outputs[output]->nmodes; i++) {
		if (scr->outputs[output]->crtc_info->mode ==
		    scr->outputs[output]->modes[i]->id)
			return (i);
	}
	return (-1);
}

/*
 * Finding information about gamma ramps and the connection between
 * brightness and gamma wasn't that easy. The source code of xrandr
 * helped me a lot here.
 *
 * Thanks to the author(s) of xrandr.
 */
static int
get_rgbb(dsbds_scr *scr, int output)
{
	int		i, j, size, best_chan, max_idx, last_idx[3];
	double		b, x0, x1, y0, y1, rgb[3];
	dsbds_output   *outp;
	XRRCrtcGamma   *gamma;
	unsigned short *rp[3];

	if (output < 0 || output > scr->noutputs)
		return (-1);
	outp = scr->outputs[output];
	if (outp->output_info->connection != RR_Connected)
		return (-1);
	size  = XRRGetCrtcGammaSize(scr->display, outp->output_info->crtc);
	gamma = XRRGetCrtcGamma(scr->display, outp->output_info->crtc);
	/*
	 * The values of the gamma ramp represent a gamma curve. Each value y
	 * of the curve is ideally defined as
	 *
	 *   f(x) = y = b * x^g
	 *
	 * where b is brightness, g is the gamma parameter, and 0 <= x <= 1.
	 *
	 * Calculating gamma:
	 *
	 *   g = [ln(y) - ln(b)] / ln(x)
	 *
	 * Calculating brightness:
	 *
	 *   Ideally, for differet x0, x1 > 0 and f(x0) = y0, f(x1) = y1 we get
	 *
	 *     g = [ln(y1) - ln(b)] / ln(x1)
	 *     g = [ln(y0) - ln(b)] / ln(x0)
	 *
	 *   Hence 
	 *
	 *     [ln(y1) - ln(b)] / ln(x1) = (ln(y0) - ln(b)) / ln(x0)
	 *
	 *   This leads to
	 *
	 *     b = exp([ln(y1)ln(x0) - ln(y0)ln(x1)] / (ln(x0) - ln(x1)))
	 */
	rp[0] = gamma->red; rp[1] = gamma->green; rp[2] = gamma->blue;

	best_chan = max_idx = 0;
	for (i = 0; i < 3; i++) {
		for (j = size - 1; j > 0; j--) {
			if (rp[i][j] < maxval)
				break;
		}
		last_idx[i] = j;
		if (j > max_idx) {
			max_idx   = j;
			best_chan = i;
		}
	}
	x0 = (double)(max_idx / 2 + 1) / (double)size;
	y0 = (double)rp[best_chan][max_idx / 2] / maxval;
	x1 = (double)(max_idx + 1) / (double)size;
	y1 = (double)rp[best_chan][max_idx] / maxval;
	if (best_chan + 1 == size) {
		b = y1;
	} else {
		b = exp((log(y1) * log(x0) - log(y0) * log(x1)) /
		    log(x0 / x1));
	}
	if (b > 1.0)
		b = 1.0;
	for (i = 0; i < 3; i++) {
		rgb[i] = log((double)(rp[i][last_idx[i] / 2]) /
			 (double)maxval / b) /
			 log((double)((last_idx[i] / 2) + 1) / (double)size);
	}
	scr->outputs[output]->red	 = rgb[0];
	scr->outputs[output]->green	 = rgb[1];
	scr->outputs[output]->blue	 = rgb[2];
	scr->outputs[output]->brightness = b;

	return (0);
}

double
dsbds_get_brightness(dsbds_scr *scr, int output)
{
	return (scr->outputs[output]->brightness);
}

void
dsbds_get_gamma(dsbds_scr *scr, int output, double *r, double *g, double *b)
{
	*r = scr->outputs[output]->red;
	*g = scr->outputs[output]->green;
	*b = scr->outputs[output]->blue;
}

int
dsbds_set_gamma_chan(dsbds_scr *scr, int output, int chan, double val)
{
	int		j, size;
	double		gval;
	XRRCrtcGamma   *gamma, *cur_gamma;
	unsigned short *ramp;

	cur_gamma = XRRGetCrtcGamma(scr->display,
	    scr->outputs[output]->output_info->crtc);
	size = XRRGetCrtcGammaSize(scr->display,
	    scr->outputs[output]->output_info->crtc);
	gamma = XRRAllocGamma(size);
	if (chan == 0) {
		ramp = gamma->red;
		scr->outputs[output]->red = val;
	} else if (chan == 1) {
		ramp = gamma->green;
		scr->outputs[output]->green = val;
	} else {
		ramp = gamma->blue;
		scr->outputs[output]->blue = val;
	}
	gval = 1.0 / (double)val;
	for (j = 0; j < size; j++) {
		gamma->red[j]   = cur_gamma->red[j];
		gamma->green[j] = cur_gamma->green[j];
		gamma->blue[j]  = cur_gamma->blue[j];
		ramp[j]		= (uint16_t)(maxval *
				  scr->outputs[output]->brightness *
				  pow((double)j / (double)(size - 1), gval));
	}
	XRRSetCrtcGamma(scr->display,
	    scr->outputs[output]->output_info->crtc, gamma);
	XFree(gamma);
	(void)XFlush(scr->display);

	return (0);
}

void
dsbds_set_brightness(dsbds_scr *scr, int output, double brightness)
{
	int	      j, size;
	double	      _r, _g, _b;
	XRRCrtcGamma *gamma;

	if (brightness < 0 || brightness > 1.0)
		return;
	size = XRRGetCrtcGammaSize(scr->display,
	       scr->outputs[output]->output_info->crtc);
	gamma = XRRAllocGamma(size);
	_r = 1.0 / (double)scr->outputs[output]->red;
	_g = 1.0 / (double)scr->outputs[output]->green;
	_b = 1.0 / (double)scr->outputs[output]->blue;

	for (j = 0; j < size; j++) {
		gamma->red[j]   = (uint16_t)(brightness *
				  pow((double)j / (double)(size - 1), _r) *
				  maxval);
		gamma->green[j] = (uint16_t)(brightness *
				  pow((double)j / (double)(size - 1), _g) *
				  maxval);
		gamma->blue[j]  = (uint16_t)(brightness *
				  pow((double)j / (double)(size - 1), _b) *
				  maxval);
	}
	XRRSetCrtcGamma(scr->display,
	    scr->outputs[output]->output_info->crtc, gamma);
	scr->outputs[output]->brightness = brightness;
	XFree(gamma);
	(void)XFlush(scr->display);
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
	int i;
	
	for (i = 0; i < 3; i++) {
		if (dsbds_set_gamma_chan(scr, output, i, gamma) < 0)
			return (-1);
	}
	return (0);
}

#if 0
int
dsbds_set_off(dsbds_scr *scr, int output)
{
	if (!dsbds_connected(scr, output))
		return (-1);
}
#endif

int
dsbds_get_lcd_brightness_level(dsbds_scr *scr, int output)
{
	int    i, val;
	char   oid[sizeof(LCD_BRIGHTNESS_OID)];
	size_t sz;

	if (!dsbds_is_lvds(scr, output))
		return (-1);
	sz = sizeof(int);
	(void)snprintf(oid, sizeof(oid), LCD_BRIGHTNESS_OID,
	    scr->outputs[output]->lvds_info->unit);
	if (sysctlbyname(oid, &val, &sz, NULL, 0) == -1)
		return (-1);
	for (i = 0; i < scr->outputs[output]->lvds_info->nlevels; i++) {
		if (scr->outputs[output]->lvds_info->levels[i] == val)
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
	    scr->outputs[output]->lvds_info->unit);
	if (sysctlbyname(oid, NULL, NULL,
	    &scr->outputs[output]->lvds_info->levels[level],
	    sizeof(int)) == -1)
		return (-1);
	return (0);
}

int
dsbds_lcd_brightness_count(dsbds_scr *scr, int output)
{
	if (!dsbds_is_lvds(scr, output))
		return (-1);
	return (scr->outputs[output]->lvds_info->nlevels);
}

static dsbds_lvds_info *
init_lvds_info(int unit)
{
	int    *levels;
	size_t	nlevels;
	dsbds_lvds_info *lvds_info;

	levels = get_lcd_brightness_levels(unit, &nlevels);
	if (levels == NULL)
		return (NULL);
	if ((lvds_info = malloc(sizeof(dsbds_lvds_info))) == NULL)
		return (NULL);
	lvds_info->unit = unit; lvds_info->levels = levels;
	lvds_info->nlevels = nlevels;

	return (lvds_info);
}

static int *
get_lcd_brightness_levels(int lcd, size_t *nlevels)
{
	int    buf[100], *levels, i, tmp;
	char   oid[sizeof(LCD_LEVELS_OID)];
	bool   sorted;
	size_t n;

	n = sizeof(buf);
	(void)snprintf(oid, sizeof(oid), LCD_LEVELS_OID, lcd);
	if (sysctlbyname(oid, buf, &n, NULL, 0) == -1)
		return (NULL);
	for (sorted = false; !sorted;) {
		sorted = true;
		for (i = 0; i < (int)(n / sizeof(int) - 1); i++) {
			if (buf[i] > buf[i + 1]) {
				sorted = false;
				tmp = buf[i]; buf[i] = buf[i + 1];
				buf[i + 1] = tmp;
			}
		}
	}
	if ((levels = malloc(n)) == NULL)
		return (NULL);
	*nlevels = n / sizeof(int);
	for (i = 0; i < (int)*nlevels; i++)
		levels[i] = buf[i];

	return (levels);
}

static XRRModeInfo **
get_modes(dsbds_scr *scr, int output, int *nmodes)
{
	int	      i, j, n;
	RRMode	      mode;
	XRRModeInfo **modev;

	modev = malloc(scr->outputs[output]->output_info->nmode *
	    sizeof(XRRModeInfo **));
	if (modev == NULL)
		return (NULL);
	for (i = n = 0; i < scr->outputs[output]->output_info->nmode; i++) {
		mode = scr->outputs[output]->output_info->modes[i];
		for (j = 0; j < scr->xrr_scr->nmode; j++) {
			if (mode != scr->xrr_scr->modes[j].id)
				continue;
			modev[n++] = &scr->xrr_scr->modes[j];
		}
	}
	*nmodes = n;

	return (modev);
}

static int
update_output(dsbds_scr *scr, int output)
{
	XRRCrtcInfo   *crtc_info;
	XRROutputInfo *output_info;

	output_info = XRRGetOutputInfo(scr->display, scr->xrr_scr,
	    scr->xrr_scr->outputs[output]);
	crtc_info = XRRGetCrtcInfo(scr->display, scr->xrr_scr,
	    scr->outputs[output]->output_info->crtc);
	if (output_info == NULL || crtc_info == NULL) {
		if (output_info != NULL)
			XRRFreeOutputInfo(output_info);
		if (crtc_info != NULL)
			XRRFreeCrtcInfo(crtc_info);
		return (-1);
	}
	XRRFreeOutputInfo(scr->outputs[output]->output_info);
	XRRFreeCrtcInfo(scr->outputs[output]->crtc_info);
	scr->outputs[output]->output_info  = output_info;
	scr->outputs[output]->crtc_info = crtc_info;

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
			    dsbds_get_mode(scr, i),
			    dsbds_get_lcd_brightness_level(scr, i),
			    scr->outputs[i]->red, scr->outputs[i]->green,
			    scr->outputs[i]->blue,
			    dsbds_output_name(scr, i));
		} else {
			ret = fprintf(fp,
			    "%s -b %f -m %d -g %f:%f:%f %s\n",
			    PATH_BACKEND,
			    dsbds_get_brightness(scr, i),
			    dsbds_get_mode(scr, i),
			    scr->outputs[i]->red, scr->outputs[i]->green,
			    scr->outputs[i]->blue,
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

