#include <QVBoxLayout>
#include <QDebug>

#include "onoff.h"

OnOff::OnOff(const QString &title, dsbds_scr *scr, int output, QWidget *parent)
	: QGroupBox(title, parent)
{
	this->scr = scr;
	this->cb  = new QCheckBox(tr("Enable output"));
	this->output = output;
	QVBoxLayout *layout = new QVBoxLayout(parent);
	
	layout->addWidget(cb);
	cb->setCheckState(dsbds_enabled(scr, output) ? \
	    Qt::Checked : Qt::Unchecked);
	connect(cb, SIGNAL(stateChanged(int)), this, SLOT(enableOutput(int)));
	setLayout(layout);
}

void OnOff::enableOutput(int state)
{
	int ok;

	if (state == Qt::Checked)
		ok = dsbds_set_on(scr, output);
	else if (state == Qt::Unchecked)
		ok = dsbds_set_off(scr, output);
	else
		return;
	cb->setCheckState(dsbds_enabled(scr, output) ? \
	    Qt::Checked : Qt::Unchecked);
	if (ok == 0)
		emit changed();
}
