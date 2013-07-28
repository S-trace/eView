#!/bin/sh
set -v
echo "Installing GTK_parts_V$version"
echo "\$0 is '$0'"
self_name=`readlink -f $0`
echo "\$self_name is '$self_name'"
self_path=`dirname $self_name`
echo "\$self_path is '$self_path'"
echo "pwd is `pwd`"
PRGNAME=eView
cd $self_path

rm -rf temp_$PRGNAME

mkdir -p temp_$PRGNAME
cd temp_$PRGNAME

sed -e '1,/^__DATA__$/d' "$self_name" > ./temp.tar.gz
gzip -d temp.tar.gz
tar -xpf ./temp.tar
rm temp.tar

#mkdir -p /userdata/application_data/$PRGNAME

mv $PRGNAME /usr/bin/
echo '[Desktop Entry]
Name=eView
Name[ru]=eView
Comment=Enhached manga viewer and filemanager
Comment[ru]=Усовершенствованный просмотрщик манги и файл-менеджер
Exec=eView
Icon=desktop_eView.png
Type=Application
Categories=Root
StartupNotify=false
Match=
NoDisplay=false
' > /usr/share/applications/${PRGNAME}.desktop
echo '[Desktop Entry]
Name=eView
GenericName=eView
Exec=eView
Type=Application
Icon=eView
Name[ru_RU]=eView
' > /usr/local/share/applications/manual.desktop
cp *.png /usr/share/filemanager/pixmaps/
cp desktop_${PRGNAME}.png /usr/local/share/desktop/icons/${PRGNAME}.png
# if ! grep eView /usr/local/share/applications/entry.order; then
#   new_list=`grep list /usr/local/share/applications/entry.order`_eView 
#   echo '[Entry Order]'> /usr/local/share/applications/entry.order
#   echo "$new_list" >> /usr/local/share/applications/entry.order
# fi
# /usr/local/share/desktop/icons
chmod +x /usr/bin/$PRGNAME
rm -rf temp_$PRGNAME
# echo "/usr/bin/$PRGNAME&" >> /home/root/.profile

#mv ./data/* /userdata/application_data/$PRGNAME
#mv ./$PRGNAME.desktop /userdata/applications/$PRGNAME.desktop

echo "Message: $PRGNAME has been installed! use remover.sh to remove it" > /tmp/eView_message.txt
dbus-send --type=method_call --dest=com.test.reader /reader/registry com.test.reader.registry.input string:"/tmp/eView_message.txt"
messagebox "$PRGNAME has been installed! use remover.sh to remove it"
cd -
rm -rf temp_*
exit 0
__DATA__
