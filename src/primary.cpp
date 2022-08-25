#include <QVBoxLayout>

#include "primary.h"

Primary::Primary(dsbds_scr *scr, int output, QWidget *parent)
	: QWidget(parent)
{
	this->scr = scr;
	this->cb  = new QCheckBox(tr("Primary Monitor"));
	this->output = output;
	QVBoxLayout *layout = new QVBoxLayout(parent);

	layout->addWidget(cb);
	cb->setCheckState(dsbds_is_primary(scr, output) ? \
	    Qt::Checked : Qt::Unchecked);
	update();
	connect(cb, SIGNAL(clicked()), this, SLOT(setPrimary()));
	setLayout(layout);
}

void Primary::setPrimary()
{
	int ok, state;

	state = cb->checkState();
	ok = dsbds_set_primary(scr, output, state == Qt::Checked);
	cb->setCheckState(dsbds_is_primary(scr, output) ? \
	    Qt::Checked : Qt::Unchecked);
	if (ok == 0)
		emit changed();
}

void Primary::update()
{

	cb->setCheckState(dsbds_is_primary(scr, output) ? \
	    Qt::Checked : Qt::Unchecked);
	if (!dsbds_enabled(scr, output))
		cb->setEnabled(false);
	else
		cb->setEnabled(true);
}

