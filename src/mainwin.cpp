/*-
 * Copyright (c) 2018 Marcel Kaiser. All rights reserved.
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

#include <QTabWidget>
#include <QMessageBox>
#include <QStatusBar>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "mainwin.h"
#include "dpi.h"
#include "dpms.h"
#include "blanktime.h"
#include "qt-helper/qt-helper.h"

struct scr_settings_s {
	int    lcdbrightness;
	int    mode;
	double sx;
	double sy;
	double gamma[3];
	double brightness;
};

static struct settings_s {
	int    dpi;
	int    blanktime;
	int    dpms[3];
	bool   dpms_on;
	struct scr_settings_s *ss;
} settings;

MainWin::MainWin(QWidget *parent) : QMainWindow(parent) {
	Display *d = XOpenDisplay(NULL);
	if (d == NULL)
		qh_errx(0, 1, "Unable to open Display");
	if ((scr = dsbds_init_screen(d)) == NULL)
		qh_errx(0, 1, "dsbds_init_screen() failed");
	updateSettings();
	QIcon wicon	  = qh_loadIcon("video-display", NULL);
	QIcon sicon	  = qh_loadStockIcon(QStyle::SP_DialogSaveButton);
	QIcon qicon	  = qh_loadStockIcon(QStyle::SP_DialogCloseButton);
	tabs		  = new QTabWidget;
	DPMS	    *dpms = new DPMS(QString(tr("DPMS")), scr);
	Blanktime   *bt	  = new Blanktime(QString(tr("Blanktime")), scr);
	DPI	    *dpi  = new DPI(QString(tr("DPI")), scr);
	QWidget	    *w	  = new QWidget(parent);
	QHBoxLayout *bbox = new QHBoxLayout;
	QHBoxLayout *hbox = new QHBoxLayout;
	QVBoxLayout *vbox = new QVBoxLayout;
	QPushButton *save = new QPushButton(sicon, tr("&Save"));
	QPushButton *quit = new QPushButton(qicon, tr("&Quit"));

	bbox->addWidget(save, 1, Qt::AlignRight);
	bbox->addWidget(quit, 0, Qt::AlignRight);
	hbox->addWidget(dpms);
	hbox->addWidget(bt);
	hbox->addWidget(dpi);
	hbox->addStretch(1);
	vbox->addLayout(hbox);
	createOutputList();
	createTabs();
	vbox->addWidget(tabs);
	vbox->addLayout(bbox);
	w->setLayout(vbox);
	setCentralWidget(w);

	connect(save, SIGNAL(clicked()), this, SLOT(saveSlot()));
        connect(quit, SIGNAL(clicked()), this, SLOT(quitSlot()));

	setWindowTitle("Display settings");
	if (quitIcon.isNull())
		setWindowIcon(wicon);
	statusBar()->setSizeGripEnabled(true);
}

void
MainWin::updateSettings()
{
	settings.ss = new struct scr_settings_s[dsbds_output_count(scr)];
	settings.blanktime = dsbds_get_blanktime(scr);
	dsbds_get_dpms_info(scr, &settings.dpms_on,
	    &settings.dpms[0], &settings.dpms[1], &settings.dpms[2]);
	settings.dpi = dsbds_get_dpi(scr);
	for (int i = 0; i < dsbds_output_count(scr); i++) {
		if (!dsbds_connected(scr, i))
			continue;
		settings.ss[i].mode	     = dsbds_get_mode(scr, i);
		settings.ss[i].brightness    = dsbds_get_brightness(scr, i);
		settings.ss[i].lcdbrightness =
		    dsbds_get_lcd_brightness_level(scr,i);
		dsbds_get_gamma(scr, i, &settings.ss[i].gamma[0],
		    &settings.ss[i].gamma[1], &settings.ss[i].gamma[2]);
		settings.ss[i].sx = dsbds_get_xscale(scr, i);
		settings.ss[i].sy = dsbds_get_yscale(scr, i);
	}
}

void
MainWin::createOutputList()
{
	for (int i = 0; i < dsbds_output_count(scr); i++) {
		Output *op = new Output(scr, i, this);
		outputs.append(op);
		connect(op, SIGNAL(changed()), this, SLOT(updateOutputs()));
	}
}

void
MainWin::createTabs()
{
	int idx = -1;

	for (int i = 0; i < outputs.count(); i++) {
		QString label(dsbds_output_name(scr, i));
		if (!dsbds_connected(scr, i))
			outputs.at(i)->setEnabled(false);
		tabs->addTab(outputs.at(i), label);
		if (idx == -1 && dsbds_connected(scr, i))
			idx = i;
	}
	tabs->setCurrentIndex(idx == -1 ? 0 : idx);
}

void
MainWin::closeEvent(QCloseEvent * /* event */)
{
	quitSlot();
}

void
MainWin::quit()
{
	QApplication::quit();
}

void
MainWin::updateOutputs()
{
	for (int i = 0; i < outputs.count(); i++)
		outputs.at(i)->update();
}

void
MainWin::saveSlot()
{
	if (dsbds_save_settings(scr) == -1)
		qh_errx(this, EXIT_FAILURE, "Couldn't save settings.");
	updateSettings();
	statusBar()->showMessage(tr("Saved"), 5000);
}

void
MainWin::quitSlot()
{
	int    dpms[3];
	bool   dpms_on, changed;
	double gamma[3];

	changed = false;
	dsbds_get_dpms_info(scr, &dpms_on, &dpms[0], &dpms[1], &dpms[2]);
	if (settings.blanktime != dsbds_get_blanktime(scr) ||
	    settings.dpms_on != dpms_on || settings.dpms[0] != dpms[0] ||
	    settings.dpms[1] != dpms[1] || settings.dpms[2] != dpms[2] ||
	    settings.dpi != dsbds_get_dpi(scr))
		changed = true;
	for (int i = 0; i < dsbds_output_count(scr); i++) {
		if (!dsbds_connected(scr, i))
			continue;
		dsbds_get_gamma(scr, i, &gamma[0], &gamma[1], &gamma[2]);
		if (settings.ss[i].brightness !=
		    dsbds_get_brightness(scr, i) ||
		    dsbds_get_lcd_brightness_level(scr, i) !=
			settings.ss[i].lcdbrightness    ||
		    settings.ss[i].gamma[0] != gamma[0] ||
		    settings.ss[i].gamma[1] != gamma[1] ||
		    settings.ss[i].gamma[2] != gamma[2] ||
		    settings.ss[i].mode	    != dsbds_get_mode(scr, i)   ||
		    settings.ss[i].sx       != dsbds_get_xscale(scr, i) ||
		    settings.ss[i].sy       != dsbds_get_yscale(scr, i)) {
			changed = true;
		}
	}
	if (!changed)
		quit();
	QMessageBox msgBox(this);
	msgBox.setWindowModality(Qt::WindowModal);
	msgBox.setText(tr("Settings have been changed"));
	msgBox.setWindowTitle(tr("Settings changed"));
	msgBox.setInformativeText(tr("Do you want to save your changes?"));
	msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
	    QMessageBox::Cancel);

	msgBox.setButtonText(QMessageBox::Save, tr("&Save"));
	msgBox.setButtonText(QMessageBox::Discard,
	    tr("&Quit without saving"));
	msgBox.setButtonText(QMessageBox::Cancel, tr("&Cancel"));
	msgBox.setDefaultButton(QMessageBox::Save);
	msgBox.setIcon(QMessageBox::Question);
	QIcon icon = qh_loadStockIcon(QStyle::SP_MessageBoxQuestion);
	msgBox.setWindowIcon(icon);

	switch (msgBox.exec()) {
	case QMessageBox::Save:
		saveSlot();
		quit();
	case QMessageBox::Discard:
		quit();
	}
}

