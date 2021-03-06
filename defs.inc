PROGRAM = dsbds
BACKEND = $${PROGRAM}_backend

isEmpty(PREFIX) {
	PREFIX=/usr/local
}

isEmpty(DATADIR) {
	DATADIR=$${PREFIX}/share/$${PROGRAM}
}

BACKEND_INSTALL_DIR = $${PREFIX}/libexec
APPSDIR		    = $${PREFIX}/share/applications
PATH_BACKEND	    = $${BACKEND_INSTALL_DIR}/$${BACKEND}
PATH_SCRIPT	    = $${PROGRAM}.sh
PATH_XRANDR	    = $${PREFIX}/bin/xrandr
PATH_XDPYINFO	    = $${PREFIX}/bin/xdpyinfo
INSTALL_SETUID	    = install -s -g wheel -m 4750 -o root

DEFINES += PATH_BACKEND=\\\"$${PATH_BACKEND}\\\"
DEFINES += PROGRAM=\\\"$${PROGRAM}\\\"
DEFINES += LOCALE_PATH=\\\"$${DATADIR}\\\"
DEFINES += PATH_SCRIPT=\\\"$${PATH_SCRIPT}\\\"
DEFINES += PATH_XRANDR=\\\"$${PATH_XRANDR}\\\"
DEFINES += PATH_XDPYINFO=\\\"$${PATH_XDPYINFO}\\\"
