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
#include <QPushButton>
#include <QCheckBox>

#include "dpi.h"

DPI::DPI(const QString &title, dsbds_scr *scr, QWidget *parent)
	: QGroupBox(title, parent)
{
	this->scr = scr;
	dpi = dsbds_get_dpi(scr);

	sb	  	  = new QSpinBox;
	QPushButton *bt	  = new QPushButton(tr("Change DPI"));
	QLabel *label	  = new QLabel("DPI");
	QHBoxLayout *hbox = new QHBoxLayout(parent);

	sb->setMinimum(1);
	sb->setMaximum(1000);
	sb->setSuffix(" dpi");
	sb->setValue(dpi);

	hbox->addWidget(label);
	hbox->addWidget(sb);
	hbox->addWidget(bt);
	hbox->addStretch(1);
	connect(bt, SIGNAL(clicked()), this, SLOT(setDPI()));

	setLayout(hbox);
}

void DPI::setDPI()
{
	dsbds_set_dpi(scr, sb->value());
	dpi = dsbds_get_dpi(scr);
}

