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
#ifndef __LIBDSBDS_H_
#define __LIBDSBDS_H_ 1

#include <stdbool.h>
#include <X11/Xlib.h>

typedef struct dsbds_scr_s dsbds_scr;

__BEGIN_DECLS
extern int    dsbds_get_blanktime(dsbds_scr *);
extern int    dsbds_get_lcd_brightness_level(dsbds_scr *, int);
extern int    dsbds_get_mode(dsbds_scr *, int);
extern int    dsbds_update_screen(dsbds_scr *);
extern int    dsbds_get_modeinfo(dsbds_scr *, int, int, char ** const mode, double *);
extern int    dsbds_get_output_count(void);
extern int    dsbds_lcd_brightness_count(dsbds_scr *, int);
extern int    dsbds_get_lcd_brightness(dsbds_scr *, int);
extern int    dsbds_mode_count(dsbds_scr *, int);
extern int    dsbds_output_count(dsbds_scr *);
extern int    dsbds_set_blue(dsbds_scr *, int, double);
extern int    dsbds_set_gamma_chan(dsbds_scr *, int, int, double);
extern int    dsbds_set_gamma(dsbds_scr *, int, double);
extern int    dsbds_set_green(dsbds_scr *, int, double);
extern int    dsbds_set_off(dsbds_scr *, int);
extern int    dsbds_set_on(dsbds_scr *, int);
extern int    dsbds_set_lcd_brightness_level(dsbds_scr *, int, int);
extern int    dsbds_set_mode(dsbds_scr *, int, int);
extern int    dsbds_set_red(dsbds_scr *, int, double);
extern int    dsbds_set_scale(dsbds_scr *scr, int output, double, double);
extern int    dsbds_save_settings(dsbds_scr *);
extern int    dsbds_get_dpi(dsbds_scr *);
extern int    dsbds_set_dpi(dsbds_scr *, int);
extern int    dsbds_set_primary(dsbds_scr *scr, int output, bool);
extern bool   dsbds_connected(dsbds_scr *, int);
extern bool   dsbds_enabled(dsbds_scr *, int);
extern bool   dsbds_is_panel(dsbds_scr *, int);
extern bool   dsbds_is_primary(dsbds_scr *, int);
extern void   dsbds_get_dpms_info(dsbds_scr *, bool *, int *, int *, int *);
extern void   dsbds_get_gamma(dsbds_scr *, int, double *, double *, double *);
extern void   dsbds_set_blanktime(dsbds_scr *, int);
extern void   dsbds_set_brightness(dsbds_scr *, int, double);
extern void   dsbds_set_dpms_enabled(dsbds_scr *, bool);
extern void   dsbds_set_dpms_timeouts(dsbds_scr *, int, int, int);
extern char   *dsbds_exec_backend(const char *, int *);
extern double dsbds_get_rate(dsbds_scr *, int, int);
extern double dsbds_get_brightness(dsbds_scr *, int);
extern double dsbds_get_xscale(dsbds_scr *, int);
extern double dsbds_get_yscale(dsbds_scr *, int);
extern dsbds_scr  *dsbds_init_screen(Display *);
extern const char *dsbds_output_name(dsbds_scr *scr, int);
__END_DECLS
#endif

