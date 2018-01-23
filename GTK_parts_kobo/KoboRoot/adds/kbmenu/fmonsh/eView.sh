#!/bin/sh
INSTALL_PATH="/mnt/onboard/eView"
CHROOT_PATH="/tmp/eView_chroot"
mkdir -p "${CHROOT_PATH}"
mount "${INSTALL_PATH}/GTK_parts.img" "${CHROOT_PATH}"
cd "${CHROOT_PATH}"
mount -o rbind /dev dev
mount -o rbind /proc proc
mount -o rbind /sys sys
mount -o bind /mnt/onboard mnt/onboard
mount -o bind /mnt/sd mnt/sd
mount -o bind "${INSTALL_PATH}/programm" root
chroot . /root/eView
