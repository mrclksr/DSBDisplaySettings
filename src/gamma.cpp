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

#include <QLabel>
#include <QGridLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <err.h>
#include <math.h>

#include "gamma.h"

Gamma::Gamma(const QString &title, dsbds_scr *scr, int output, QWidget *parent)
	: QGroupBox(title, parent)
{
	this->scr = scr;
	this->output = output;

	dsbds_get_gamma(scr, output, &red, &green, &blue);
	redSl   = new Slider(Qt::Vertical, QString("Red"),   0, 100,
			    (int)(red * 10), 10);
	greenSl = new Slider(Qt::Vertical, QString("Green"), 0, 100,
			    (int)(green * 10), 10);
	blueSl  = new Slider(Qt::Vertical, QString("Blue"),  0, 100, 
			    (int)(blue * 10), 10);
	gammaSl = new Slider(Qt::Vertical, QString("Gamma"), 0, 100,
			    (int)(red * 10), 10);
	QHBoxLayout *hbox = new QHBoxLayout;

	hbox->addWidget(redSl);
	hbox->addWidget(greenSl);
	hbox->addWidget(blueSl);
	hbox->addWidget(gammaSl);
	setLayout(hbox);

	connect(redSl, SIGNAL(valChanged(int)),   this, SLOT(setRed(int)));
	connect(greenSl, SIGNAL(valChanged(int)), this, SLOT(setGreen(int)));
	connect(blueSl, SIGNAL(valChanged(int)),  this, SLOT(setBlue(int)));
	connect(gammaSl, SIGNAL(valChanged(int)), this, SLOT(setGamma(int)));
}

void Gamma::update()
{
	dsbds_get_gamma(scr, output, &red, &green, &blue);
	redSl->setVal((int)(red * 10));
	greenSl->setVal((int)(green * 10));
	blueSl->setVal((int)(blue * 10));
}

void Gamma::setGamma(int val)
{
	dsbds_set_gamma(scr, output, (double)val / 10.0);
	redSl->setVal(val);
	greenSl->setVal(val);
	blueSl->setVal(val);
}

void Gamma::setRed(int val)
{
	dsbds_set_red(scr, output, (double)val / 10.0);
	emit changed();
	
}

void Gamma::setGreen(int val)
{
	dsbds_set_green(scr, output, (double)val / 10.0);
	emit changed();
}

void Gamma::setBlue(int val)
{
	dsbds_set_blue(scr, output, (double)val / 10.0);
	emit changed();
}

