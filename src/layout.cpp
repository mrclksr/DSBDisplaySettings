/*-
 * Copyright (c) 2023 Marcel Kaiser. All rights reserved.
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
#include <QStyle>
#include <QGraphicsView>
#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "layout.h"
#include "outputrect.h"

Layout::Layout(dsbds_scr *scr, QWidget *parent) : QWidget(parent) {
	this->scr = scr;
	scene = new QGraphicsScene;
	QGraphicsView *widget = new QGraphicsView;
	QIcon applyIcon = style()->standardIcon(
				   QStyle::SP_DialogApplyButton, 0, this);
	QPushButton *apply = new QPushButton(applyIcon, tr("&Apply"), this);
	widget->setScene(scene);
	widget->centerOn(0, 0);

	QHBoxLayout *hbox = new QHBoxLayout;
	QVBoxLayout *vbox = new QVBoxLayout;

	vbox->addWidget(widget);
	hbox->addWidget(apply, 0, Qt::AlignRight);
	vbox->addLayout(hbox);
	setLayout(vbox);
	update();
	connect(apply, SIGNAL(clicked()), this, SLOT(changeLayout()));
}

void Layout::emitChanged()
{
	emit changed();
}

void Layout::update()
{
	int   rgb = 60;
	qreal width, height;

	for (const auto &item: scene->items())
		delete item;
	scene->clear();
	outrects.clear();
	for (int i = width = height = 0; i < dsbds_output_count(scr); i++) {
		if (!dsbds_enabled(scr, i))
			continue;
		QColor color(rgb, rgb, rgb, 180);
		rgb += 30; rgb %= 256;
		if (rgb < 60)
			rgb = 60;
		width += (qreal)dsbds_output_width(scr, i);
		height += (qreal)dsbds_output_height(scr, i);
		OutputRect *rect = new OutputRect(scr, i, scaleFactor, color);
		outrects.append(rect);
	}
	scene->setSceneRect(0, 0,
	    width * scaleFactor, height * scaleFactor);
	QGraphicsRectItem *screenRect = new QGraphicsRectItem();
	screenRect->setPen(Qt::NoPen);
	screenRect->setBrush(Qt::black);
	screenRect->setRect(0, 0, width * scaleFactor, height * scaleFactor);
	scene->addItem(screenRect);
	for (const auto &item: outrects)
		scene->addItem(item);
}

void Layout::changeLayout(void)
{
	for (const auto &item: outrects) {
		QPointF point = item->realPos();
		dsbds_set_pos(scr, item->output, point.x(), point.y());
	}
}

