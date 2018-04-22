include(../../defs.inc)

TEMPLATE     = app
TARGET	     = $${BACKEND}
SOURCES	    += dsbds_backend.c ../libdsbds.c ../dsbcfg/dsbcfg.c
LIBS        += -lX11 -lXxf86vm -lXext -lXrandr
INCLUDEPATH += lib .

isEmpty(PORTS) {
	target.path	= $${BACKEND_INSTALL_DIR}
	target.commands = $${INSTALL_SETUID} $${BACKEND} $${BACKEND_INSTALL_DIR}
} else {
	target.path	= $${BACKEND_INSTALL_DIR}
	target.files	= $${BACKEND}
}
INSTALLS = target
QMAKE_POST_LINK=$(STRIP) $(TARGET)
