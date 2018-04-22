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

#include <QVBoxLayout>
#include <QString>
#include <QLabel>
#include <err.h>

#include "slider.h"

Slider::Slider(Qt::Orientation orientation, const QString &name,
    int min, int max, int val, int scale, QWidget *parent)
	: QGroupBox(name, parent)
{
	scalef = scale;
	QVBoxLayout *layout = new QVBoxLayout(parent);
	slider = new QSlider(orientation);
	slider->setMinimum(min);
	slider->setMaximum(max);
	if (orientation == Qt::Horizontal)
		slider->setTickPosition(QSlider::TicksBelow);
	else
		slider->setTickPosition(QSlider::TicksLeft);
	setVal(val);

	if (orientation == Qt::Horizontal)
		layout->addWidget(slider, 0, Qt::AlignVCenter);
	else
		layout->addWidget(slider, 0, Qt::AlignHCenter);
	setLayout(layout);

	connect(slider, SIGNAL(valueChanged(int)), this,
	    SLOT(emitValChanged(int)));
}

void Slider::emitValChanged(int val)
{
	value = val;
	slider->setValue(val);
	sliderSetToolTip(val);
	emit valChanged(val);
}

void Slider::sliderSetToolTip(int val)
{
	QString str;

	if (scalef > 1)
		str = QString("%1").arg((float)val / scalef, 3, 'f', 1);
	else
		str = QString("%1").arg(val);
	slider->setToolTip(str);
}

void Slider::setVal(int val)
{
	value = val;
	slider->setValue(val);
	sliderSetToolTip(val);
}

