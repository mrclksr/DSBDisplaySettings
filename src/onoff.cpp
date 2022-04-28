#include <QVBoxLayout>

#include "onoff.h"

OnOff::OnOff(dsbds_scr *scr, int output, QWidget *parent)
	: QWidget(parent)
{
	this->scr = scr;
	this->cb  = new QCheckBox(tr("Enable output"));
	this->output = output;
	QVBoxLayout *layout = new QVBoxLayout(parent);
	
	layout->addWidget(cb);
	cb->setCheckState(dsbds_enabled(scr, output) ? \
	    Qt::Checked : Qt::Unchecked);
	update();
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

void OnOff::update()
{
	int nenabled = 0;

	cb->setCheckState(dsbds_enabled(scr, output) ? \
	    Qt::Checked : Qt::Unchecked);

	//
	// Don't allow to disable the only enabled output
	//
	for (int i = 0; i < dsbds_output_count(scr); i++) {
		if (dsbds_enabled(scr, i))
			nenabled++;
	}
	if (nenabled < 2 && dsbds_enabled(scr, output))
		cb->setEnabled(false);
	else
		cb->setEnabled(true);
}
