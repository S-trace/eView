#!/bin/sh
# Clean up
umount /tmp/chroot/dev/pts
umount /tmp/chroot/dev
umount /tmp/chroot/proc
umount /tmp/chroot/sys
rm /root/.eView /root/eView/.eView /var/cache/fontconfig/* /var/lib/xkb/* /var/log/Xorg.0.log* /tmp/* /.eView /tmp/chroot -rf

# Trace app startup
strace -ffFvs9999 -o /tmp/trace sh -c /root/eView/eView064t2_russian.app
rm /root/.eView /.eView /root/eView/.eView /var/cache/fontconfig/* /var/lib/xkb/* /var/log/Xorg.0.log* -rf

# Dynamically loaded files, not caught by strace
cat > /tmp/trace.ff <<EOF
/root/eView/eView064t2_russian.app
/lib/ld-linux-armhf.so.3
/lib/ld-linux.so.3
/lib/arm-linux-gnueabihf/ld-linux.so.3
/lib/arm-linux-gnueabihf/ld-linux-armhf.so.3
/usr/share/themes/Sato/index.theme
/usr/share/themes/Sato/gtk-2.0/Images/Scrollbars/trough-scrollbar-vert.png
/usr/share/themes/Sato/gtk-2.0/Images/Scrollbars/stepper-insens.png
/usr/share/themes/Sato/gtk-2.0/Images/Scrollbars/stepper.png
/usr/share/themes/Sato/gtk-2.0/Images/Scrollbars/trough-scrollbar-horiz.png
/usr/share/themes/Sato/gtk-2.0/Images/Titlebar/titlebar-panel-background.png
/usr/share/themes/Sato/gtk-2.0/Images/Titlebar/titlebar-panel-applet-background.png
/usr/share/themes/Sato/gtk-2.0/gtkrc
/usr/share/themes/Sato/matchbox/dia-right-edge.png
/usr/share/themes/Sato/matchbox/theme.xml
/usr/share/themes/Sato/matchbox/titlebar-right-edge.png
/usr/share/themes/Sato/matchbox/titlebar-left-edge.png
/usr/share/themes/Sato/matchbox/dia-left-edge.png
/usr/share/themes/Sato/matchbox/close-button-active.png
/usr/share/themes/Sato/matchbox/arrow-down-active.png
/usr/share/themes/Sato/matchbox/dia-tile.png
/usr/share/themes/Sato/matchbox/close-button.png
/usr/share/themes/Sato/matchbox/titlebar-tile.png
/usr/share/themes/Sato/matchbox/arrow-down.png
/usr/bin/strace
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-tga.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-wbmp.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-png.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-xpm.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-icns.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-jpeg.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-qtif.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-ras.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-pcx.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-jasper.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-bmp.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-ani.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-gif.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-pnm.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-ico.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-tiff.so
/usr/lib/arm-linux-gnueabihf/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-xbm.so
EOF

# Prepare skeleton
mkdir /tmp/chroot/
(
  cd /tmp/chroot/ || exit 1
  mkdir -p proc sys dev var/log tmp mnt/onboard mnt/sd root
)
cd /

# Parse strace output and copy all files to /tmp/chroot/
cat /tmp/trace.*|tr '"' '\n'|sort -u|while read -r file; do
  if [ -e "$file" ] && ! [ -d "$file" ]; then
    echo "$file"
    readlink -f "$file"
  fi
done|sort -u|grep -Ev '/dev/|/sys/|/proc/|/var/log/Xorg.0.log|/var/cache/fontconfig/|/var/lib/xkb/|/root/eView/.eView/|FairyTail01.zip'|tee /tmp/final.list|cpio --pass-through --make-directories /tmp/chroot

# Build cpio archive
cd /tmp/chroot
find|cpio -o -H newc > /tmp/chroot_full.cpio

# Prepare ext4 image
dd if=/dev/zero bs=1048576 count=64 of=GTK_Parts.img
mkdir /tmp/recoveryfs
mount /dev/mmcblk0p2 /tmp/recoveryfs
yes|/tmp/recoveryfs/sbin/mkfs.ext4 -L GTK_Parts /tmp/GTK_Parts.img
umount /tmp/recoveryfs
rmdir /tmp/recoveryfs

# Build ext4 image
mkdir mnt
mount /tmp/GTK_Parts.img mnt
cd mnt
cpio -id < /tmp/chroot_full.cpio
cd ..
umount mnt

# Test it right now
mount -o rbind /dev dev
mount -o rbind /proc proc
mount -o rbind /sys sys
chroot . /root/eView/eView064t2_russian.app
