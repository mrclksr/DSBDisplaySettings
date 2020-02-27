/*-
 * Copyright (c) 2020 Marcel Kaiser. All rights reserved.
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
#include <QCheckBox>
#include <QPushButton>

#include "scale.h"

Scale::Scale(const QString &title, dsbds_scr *scr, int output, QWidget *parent)
	: QGroupBox(title, parent)
{
	this->scr    = scr;
	this->output = output;
	this->lock   = true;

	xscale = dsbds_get_xscale(scr, output);
	yscale = dsbds_get_yscale(scr, output);

	sx		   = new QDoubleSpinBox;
	sy		   = new QDoubleSpinBox;
	QCheckBox      *cb = new QCheckBox(QString(tr("=")));
	QPushButton    *pb = new QPushButton(QString(tr("Scale")));
	QHBoxLayout *hbox  = new QHBoxLayout;

	cb->setTristate(false);
	cb->setCheckState(Qt::Checked);

	sx->setMinimum(0.00);
	sx->setValue(xscale);
	sx->setSingleStep(0.01);
	sx->setDecimals(3);
	sx->setSuffix(" X");

	sy->setMinimum(0);
	sy->setValue(yscale);
	sy->setSingleStep(0.01);
	sy->setDecimals(3);
	sy->setSuffix(" Y");

	hbox->addWidget(sx, 0, Qt::AlignLeft);
	hbox->addWidget(cb, 0, Qt::AlignCenter);
	hbox->addWidget(sy, 0, Qt::AlignLeft);
	hbox->addWidget(pb, 0, Qt::AlignLeft);
	hbox->addStretch(1);
	setLayout(hbox);

	connect(sx, SIGNAL(valueChanged(double)), this, SLOT(setXScale(double)));
	connect(sy, SIGNAL(valueChanged(double)), this, SLOT(setYScale(double)));
	connect(pb, SIGNAL(clicked()), this, SLOT(doScale()));
	connect(cb, SIGNAL(stateChanged(int)), this, SLOT(changeLockState(int)));
}

void Scale::setXScale(double value)
{
	xscale = value;
	if (this->lock) {
		yscale = value;
		sy->setValue(value);
	}
}

void Scale::setYScale(double value)
{
	yscale = value;
	if (this->lock) {
		xscale = value;
		sx->setValue(value);
	}
}

void Scale::doScale()
{
	dsbds_set_scale(scr, output, xscale, yscale);
}

void Scale::changeLockState(int status)
{
	this->lock = status == Qt::Unchecked ? false : true;
}
