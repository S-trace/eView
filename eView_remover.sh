#!/bin/sh 
PRGNAME=eView
rm -r /usr/bin/$PRGNAME /usr/share/applications/$PRGNAME.desktop /usr/local/share/applications/${PRGNAME}.desktop /usr/share/filemanager/pixmaps/desktop_$PRGNAME.png /usr/local/share/desktop/icons/$PRGNAME.png .$PRGNAME
echo '[Desktop Entry]
Name=Manual
GenericName=Manual
Exec=help.sh
Type=Application
Icon=manual
Name[ru_RU]=Руководство
' > /usr/local/share/applications/manual.desktop