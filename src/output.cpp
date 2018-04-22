#include "output.h"
#include "dpms.h"
#include "blanktime.h"
#include "gamma.h"
#include "brightness.h"

Output::Output(dsbds_scr *scr, int output, QWidget *parent) : QWidget(parent) {
	mode         = new Mode(QString(tr("Mode")), scr, output);
	gamma        = new Gamma(QString(tr("Gamma correction")), scr, output);
	brightness   = new Brightness(QString(tr("Brightness")), scr, output);
	this->scr    = scr;
	this->output = output;

	QGridLayout *grid = new QGridLayout;
	grid->setColumnStretch(1, 1);
	grid->setRowStretch(1, 1);
	grid->addWidget(mode,  0, 0);
	grid->addWidget(gamma,  0, 1, 2, 1);
	grid->addWidget(brightness, 1, 0);

	setLayout(grid);
}

