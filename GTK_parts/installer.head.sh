#!/bin/sh
version=3
echo "Installing GTK_parts_V$version"
echo "\$0 is '$0'"
self_name=`readlink -f $0`
echo "\$self_name is '$self_name'"
self_path=`dirname $self_name`
echo "\$self_path is '$self_path'"
echo "pwd is `pwd`"
PRGNAME=GTK_parts
rm -rf $self_path/temp_$PRGNAME
mkdir -p $self_path/temp_$PRGNAME
cd $self_path/temp_$PRGNAME
sed -e '1,/^__DATA__$/d' "$self_name" > $self_path/temp.tar.gz
gzip -d $self_path/temp.tar.gz
cd /
tar -vxpf $self_path/temp.tar
cd -
# Просто на всякий случай перегенерируем файлы
gdk-pixbuf-query-loaders > /etc/gtk-2.0/gdk-pixbuf.loaders
pango-querymodules > '/etc/pango/pango.modules'
rm -rf $self_path/temp.tar $self_path/temp_$PRGNAME
echo "Message: GTK_parts version $version installed" > /tmp/message.txt
echo $version > /home/root/.GTK_parts.version
dbus-send --type=method_call --dest=com.test.reader /reader/registry com.test.reader.registry.input string:"/tmp/message.txt"
exit 0
__DATA__
