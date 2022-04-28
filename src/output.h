/*-
 * Copyright (c) 2019 Marcel Kaiser. All rights reserved.
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

#ifndef _OUTPUT_H_
#define _OUTPUT_H_ 1
#include "dpms.h"
#include "gamma.h"
#include "brightness.h"
#include "mode.h"
#include "onoff.h"
#include "scale.h"
#include "primary.h"

class Output : public QWidget
{
	Q_OBJECT
public:
	Output(dsbds_scr *scr, int output, QWidget *parent = 0);
	void update();
signals:
	void changed();
private slots:
	void emitChanged();
private:
	int	   output;
	Mode	   *mode;
	Gamma	   *gamma;
	OnOff	   *onOff;
	Primary	   *primary;
	Scale	   *scale;
	dsbds_scr  *scr;
	QComboBox  *cbox;
	Brightness *brightness;
};
#endif
