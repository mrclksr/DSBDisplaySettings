/*-
 * Copyright (c) 2017 Marcel Kaiser. All rights reserved.
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

#include <QLabel>
#include <QGridLayout>
#include <QSpinBox>
#include <QCheckBox>
#include "qt-helper/qt-helper.h"
#include "lcdbrightness.h"

LCDBrightness::LCDBrightness(dsbds_scr *scr, int output, QWidget *parent)
	: QWidget(parent)
{
	level	     = dsbds_get_lcd_brightness_level(scr, output);
	nlevels	     = dsbds_lcd_brightness_count(scr, output);
	this->scr    = scr;
	this->output = output;
	slider	     = new Slider(Qt::Horizontal,
				  QString(tr("LCD brightness level")), 0,
				  nlevels - 1, level);
	connect(slider, SIGNAL(valChanged(int)), this,
	    SLOT(setBrightness(int)));
	QVBoxLayout *vbox = new QVBoxLayout(parent);
	vbox->addWidget(slider);
	setLayout(vbox);
}

void LCDBrightness::setBrightness(int level)
{
	int  e;
	char cmd[sizeof(PATH_BACKEND) + 50], *msg;

	(void)snprintf(cmd, sizeof(cmd) - 1, "%s -l %d %s", PATH_BACKEND,
	    level, dsbds_output_name(scr, output));
	msg = dsbds_exec_backend(cmd, &e);
	if (e == 0)
		return;
	qh_warnx(0, "Backend error: %s", msg);
	update();	
}

void LCDBrightness::update()
{
	level = dsbds_get_lcd_brightness_level(scr, output);
	slider->setVal(level);
}

