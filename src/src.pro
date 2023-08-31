include(../defs.inc)
TEMPLATE     = app
TARGET	     = ../dsbds
DEFINES	    += QT_DEPRECATED_WARNINGS
QT   	    += widgets
LIBS	    += -lX11 -lXxf86vm -lXext -lXrandr
DEPENDPATH  += . ../lib ../lib/backend
INCLUDEPATH += . ../lib

# Input
HEADERS += ../lib/libdsbds.h \
           blanktime.h \
           brightness.h \
	   swbrightness.h \
           dpms.h \
           gamma.h \
           lcdbrightness.h \
           mainwin.h \
           mode.h \
           slider.h \
	   output.h \
	   onoff.h \
	   scale.h \
	   dpi.h \
	   primary.h \
	   layout.h \
	   outputrect.h \
           ../lib/qt-helper/qt-helper.h \
	   ../lib/dsbcfg/dsbcfg.h
SOURCES += ../lib/libdsbds.c \
           blanktime.cpp \
           brightness.cpp \
	   swbrightness.cpp \
           dpms.cpp \
           gamma.cpp \
           lcdbrightness.cpp \
           main.cpp \
           mainwin.cpp \
           mode.cpp \
           output.cpp \
           slider.cpp \
	   onoff.cpp \
	   scale.cpp \
	   dpi.cpp \
	   primary.cpp \
	   layout.cpp \
	   outputrect.cpp \
	   ../lib/dsbcfg/dsbcfg.c \
           ../lib/qt-helper/qt-helper.cpp

