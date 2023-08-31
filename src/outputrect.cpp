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

#include <QGraphicsSimpleTextItem>
#include <QGraphicsScene>

#include "outputrect.h"

OutputRect::OutputRect(dsbds_scr *scr, int output, qreal scaleFactor,
	QColor color, QGraphicsItem *parent) : QGraphicsRectItem(parent)
{
	QBrush brush(Qt::SolidPattern);

	brush.setColor(color);
	setBrush(brush);
	setPen(Qt::NoPen);

	setFlags(QGraphicsItem::ItemIsMovable | \
		 QGraphicsItem::ItemSendsGeometryChanges);
	setRect(0, 0,
		(qreal)dsbds_output_width(scr, output) * scaleFactor,
		(qreal)dsbds_output_height(scr, output) * scaleFactor);
	setFlag(QAbstractGraphicsShapeItem::ItemIsMovable);
	QGraphicsSimpleTextItem *text = new QGraphicsSimpleTextItem(this);
	text->setText(dsbds_output_name(scr, output));
	text->setBrush(Qt::white);
	text->setPos(
	    (boundingRect().width() - text->boundingRect().width()) / 2,
	    (boundingRect().height() - text->boundingRect().height()) / 2);
	setPos((qreal)dsbds_get_x_pos(scr, output) * scaleFactor,
		(qreal)dsbds_get_y_pos(scr, output) * scaleFactor);
	this->output = output;
	this->scaleFactor = scaleFactor;
}

QVariant OutputRect::itemChange(GraphicsItemChange change,
	const QVariant &value)
{
	if (change != ItemPositionChange || !scene())
		return QGraphicsItem::itemChange(change, value);
	QPointF newPos = value.toPointF();
	QRectF rect = scene()->sceneRect();
	if (rect.right() < (newPos.x() + boundingRect().width()))
		newPos.setX(rect.right() - boundingRect().width());
	else if (rect.left() > newPos.x())
		newPos.setX(0);
	if (rect.bottom() < newPos.y() + boundingRect().height())
		newPos.setY(rect.bottom() - boundingRect().height());
	else if (rect.top() > newPos.y())
		newPos.setY(0);
	return newPos;
}

QPointF
OutputRect::realPos(void)
{
	QPointF point = pos();

	return (QPointF(point.x() / scaleFactor, point.y() / scaleFactor));
}

