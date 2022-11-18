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

#include <QVBoxLayout>

#include "swbrightness.h"

SWBrightness::SWBrightness(dsbds_scr *scr, int output,
	QWidget *parent) : QWidget(parent)
{
	this->scr    = scr;
	this->output = output;
	QVBoxLayout *vbox = new QVBoxLayout(parent);
	brightness = dsbds_get_brightness(scr, output);
	if (brightness <= 0)
		brightness = 0;
	slider = new Slider(Qt::Horizontal, QString(tr("Software brightness")),
				0, 100, (int)(brightness * 100), 1);
	connect(slider, SIGNAL(valChanged(int)), this,
	    SLOT(setBrightness(int)));
	vbox->addWidget(slider);
	vbox->addStretch(1);
	setLayout(vbox);
}

void SWBrightness::setBrightness(int value)
{
	dsbds_set_brightness(scr, output, (double)value / 100);
	emit changed();
}

void SWBrightness::update()
{
	brightness = dsbds_get_brightness(scr, output);
	slider->setVal((int)(brightness * 100));
}

