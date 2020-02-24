/*-
 * Copyright (c) 2019 Marcel Kaiser. All rights reserved.
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

#include "output.h"

Output::Output(dsbds_scr *scr, int output, QWidget *parent) : QWidget(parent) {
	mode         = new Mode(QString(tr("Mode")), scr, output);
	scale	     = new Scale(QString(tr("Scale")), scr, output);
	gamma        = new Gamma(QString(tr("Gamma correction")), scr, output);
	brightness   = new Brightness(QString(tr("Brightness")), scr, output);
	onOff	     = new OnOff(QString(tr("Enable/Disable")), scr, output);
	this->scr    = scr;
	this->output = output;

	QGridLayout *grid = new QGridLayout;
	grid->setColumnStretch(1, 1);
	grid->setRowStretch(1, 1);
	grid->addWidget(mode,  0, 0);
	grid->addWidget(gamma,  0, 1, 4, 1);
	grid->addWidget(scale, 1, 0);
	grid->addWidget(onOff, 2, 0);
	grid->addWidget(brightness, 3, 0);

	setLayout(grid);

	connect(onOff, SIGNAL(changed()), this, SLOT(emitChanged()));
	connect(mode, SIGNAL(changed()), this, SLOT(emitChanged()));
}

void Output::emitChanged()
{
	emit changed();
}

void Output::update()
{
	mode->update();
	brightness->update();
	gamma->update();
	onOff->update();
}
