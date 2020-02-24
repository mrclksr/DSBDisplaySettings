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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <err.h>

#include "../libdsbds.h"

static void
usage()
{
	(void)printf("Usage: dsbds_backend [-B blanktime]"	\
	    "[-d enable:standby:suspend:off]\n"			\
	    "       dsbds_backend [-b brightness][-g r:g:b]"	\
	    "[-l <LCD brightness>]\n"				\
	    "                     [-s <sx>x<sy>] output\n");
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	int	  ch, blanktime, dpms[4], i, mode, lbrightness, output;
	bool	  Bflag, bflag, dflag, gflag, lflag, mflag, sflag;
	char	  *p;
	double	  brightness, gamma[3], sx, sy;
	Display	  *d;
	dsbds_scr *scr;

	(void)setlocale(LC_NUMERIC, "C");

	Bflag = bflag = dflag = gflag = lflag = mflag = sflag = false;
	while ((ch = getopt(argc, argv, "B:b:d:g:l:m:s:")) != -1) {
		switch (ch) {
		case 'B':
			Bflag = true;
			blanktime = strtol(optarg, NULL, 10);
			break;
		case 'b':
			bflag = true;
			brightness = strtod(optarg, NULL);
			break;
		case 'd':
			dflag = true;
			for (i = 0, p = optarg;
			    ((p = strtok(p, ":"))) != NULL && i < 4; p = NULL)
				dpms[i++] = strtol(p, NULL, 10);
			if (i != 4)
				usage();
			break;
		case 'g':
			gflag = true;
			for (i = 0, p = optarg;
			    ((p = strtok(p, ":"))) != NULL && i < 3; p = NULL)
				gamma[i++] = strtod(p, NULL);
			if (i != 3)
				usage();
			break;
		case 'l':
			lflag = true;
			lbrightness = strtol(optarg, NULL, 10);
			break;
		case 'm':
			mflag = true;
			mode = strtol(optarg, NULL, 10);
			break;
		case 's':
			sflag = true;
			for (i = 0, p = optarg;
			    ((p = strtok(p, "x"))) != NULL && i < 2;
			    p = NULL, i++) {
				if (i == 0)
					sx = strtod(p, NULL);
				else
					sy = strtod(p, NULL);
			}
			if (i != 2) {
				warnx("HERE");
				usage();
			}
			break;
		case '?':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	
	if ((lflag || bflag || gflag || mflag) && argc < 1)
		usage();
	if (!lflag)
		(void)seteuid(getuid());
	if ((d = XOpenDisplay(NULL)) == NULL)
		errx(EXIT_FAILURE, "Failed to open Display");
	if ((scr = dsbds_init_screen(d)) == NULL)
		errx(EXIT_FAILURE, "Failed to init screen");
	if (argc >= 1) {
		for (output = 0; output < dsbds_output_count(scr); output++) {
			if (!strcmp(dsbds_output_name(scr, output), argv[0]))
				break;
		}
		if (output == dsbds_output_count(scr))
			errx(EXIT_FAILURE, "No such output %s", argv[0]);
	}
	if (Bflag)
		dsbds_set_blanktime(scr, blanktime);
	if (bflag) {
		dsbds_set_brightness(scr, output, brightness);
	}
	if (dflag) {
		dsbds_set_dpms_enabled(scr, dpms[0] != 0 ? true : false);
		dsbds_set_dpms_timeouts(scr, dpms[1], dpms[2], dpms[3]);
	}
	if (gflag) {
		for (i = 0; i < 3; i++)
			dsbds_set_gamma_chan(scr, output, i, gamma[i]);
	}
	if (lflag) {
		if (dsbds_set_lcd_brightness_level(scr, output,
		    lbrightness) == -1)
			errx(EXIT_FAILURE, "Failed to set LCD brightness");
		(void)seteuid(getuid());
	}
	if (mflag)
		dsbds_set_mode(scr, output, mode);
	if (sflag)
		dsbds_set_scale(scr, output, sx, sy);
	return (EXIT_SUCCESS);
}

