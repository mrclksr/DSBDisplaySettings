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
#include <QPushButton>
#include <QStringList>
#include <QHBoxLayout>

#include "mode.h"

Mode::Mode(const QString &title, dsbds_scr *scr, int output,
	QWidget *parent) : QGroupBox(title, parent)
{
	int    w, h;
	char   buf[64];
	double r;

	this->scr    = scr;
	this->output = output;
	QStringList modes = { };

	nmodes = dsbds_mode_count(scr, output);
	for (int i = 0; i < nmodes; i++) {
		dsbds_get_modeinfo(scr, output, i, &w, &h, &r);
		(void)snprintf(buf, sizeof(buf), "%dx%d @ %.2f", w, h, r);
		modes.append(buf);
	}
	cbox		  = new QComboBox();
	QHBoxLayout *hbox = new QHBoxLayout(this);
	QPushButton *pb   = new QPushButton(QString(tr("Set mode")), this);

	cbox->addItems(modes);
	hbox->addWidget(cbox);
	hbox->addWidget(pb);
	cbox->setCurrentIndex(dsbds_get_mode(scr, output));
	connect(pb, SIGNAL(pressed()), this, SLOT(setMode()));
	setLayout(hbox);
}

void Mode::setMode()
{
	dsbds_set_mode(scr, output, cbox->currentIndex());
	cbox->setCurrentIndex(dsbds_get_mode(scr, output));
	emit changed();
}

