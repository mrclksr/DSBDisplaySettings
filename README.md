ABOUT
     DSBDisplaySettings is a Qt program for FreeBSD that allows you to
     configure/change properties of your X display/video outputs. That is,
     DPMS settings, blanktime, graphics mode, software and LCD brigtness, and
     gamma correction.

INSTALLATION
   Dependencies
     DSBDisplaySettings depends on devel/qt5-buildtools, devel/qt5-core,
     devel/qt5-linguisttools, devel/qt5-qmake, x11-toolkits/qt5-gui, and
     x11-toolkits/qt5-widgets

   Building and installation
     # git clone https://github.com/mrclksr/DSBDisplaySettings.git
     # git clone https://github.com/mrclksr/dsbcfg.git

     # cd DSBDisplaySettings && qmake && make
     # make install

SETUP
     DSBDisplaySettings saves the settings in a shell script at
     ~/.config/DSB/dsbds.sh. Make this script execute on session start by
     adding the line

         ~/.config/DSB/dsbds.sh

     to your ~/.xinitrc or to your window manager's startup script.

