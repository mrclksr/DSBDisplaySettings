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

#include "dpms.h"

DPMS::DPMS(const QString &title, dsbds_scr *scr, QWidget *parent)
	: QGroupBox(title, parent)
{
	this->scr = scr;

	dsbds_get_dpms_info(scr, &enabled, &standby, &suspend, &off);

	QCheckBox *cb	    = new QCheckBox(tr("Enable DPMS"));
	QGridLayout *grid   = new QGridLayout(this);
	QSpinBox *offSb	    = new QSpinBox;
	QSpinBox *suspendSb = new QSpinBox;
	QSpinBox *standbySb = new QSpinBox;
	QLabel *offL	    = new QLabel("Off");
	QLabel *suspendL    = new QLabel("Suspend");
	QLabel *standbyL    = new QLabel("Standby");

	offSb->setMinimum(1);
	offSb->setSuffix(" min");
	offSb->setValue(off);

	suspendSb->setMinimum(1);
	suspendSb->setSuffix(" min");
	suspendSb->setValue(suspend);

	standbySb->setMinimum(1);
	standbySb->setSuffix(" min");
	standbySb->setValue(standby);

	grid->addWidget(standbyL,  0, 0);
	grid->addWidget(standbySb, 0, 1);

	grid->addWidget(suspendL,  1, 0);
	grid->addWidget(suspendSb, 1, 1);

	grid->addWidget(offL,  2, 0);
	grid->addWidget(offSb, 2, 1);

	grid->addWidget(cb, 3, 0);
	cb->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);

	connect(cb, SIGNAL(stateChanged(int)), this, SLOT(enableDPMS(int)));
	connect(standbySb, SIGNAL(valueChanged(int)), this,
	    SLOT(setStandby(int)));
	connect(suspendSb, SIGNAL(valueChanged(int)), this,
	    SLOT(setSuspend(int)));
	connect(offSb, SIGNAL(valueChanged(int)), this,
	    SLOT(setOff(int)));
	setLayout(grid);
	grid->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding,
            QSizePolicy::Expanding), 4, 0);

}

void DPMS::enableDPMS(int state)
{
	dsbds_set_dpms_enabled(scr, state == Qt::Checked ? true : false);
	dsbds_get_dpms_info(scr, &enabled, &standby, &suspend, &off);
}

void DPMS::setStandby(int value)
{
	dsbds_set_dpms_timeouts(scr, value, suspend, off);
	dsbds_get_dpms_info(scr, &enabled, &standby, &suspend, &off);
}

void DPMS::setSuspend(int value)
{
	dsbds_set_dpms_timeouts(scr, standby, value, off);
	dsbds_get_dpms_info(scr, &enabled, &standby, &suspend, &off);
}

void DPMS::setOff(int value)
{
	dsbds_set_dpms_timeouts(scr, standby, value, value);
	dsbds_get_dpms_info(scr, &enabled, &standby, &suspend, &off);
}

