#ifndef _OUTPUT_H_
#define _OUTPUT_H_ 1
#include "dpms.h"
#include "gamma.h"
#include "brightness.h"
#include "mode.h"

class Output : public QWidget
{
	Q_OBJECT
public:
	Output(dsbds_scr *scr, int output, QWidget *parent = 0);
private:
	int	   output;
	Mode	   *mode;
	Gamma	   *gamma;
	dsbds_scr  *scr;
	QComboBox  *cbox;
	Brightness *brightness;
};
#endif

