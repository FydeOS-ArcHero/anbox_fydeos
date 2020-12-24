#!/bin/bash

set -ex

ramdisk=$1
system=$2

if [ -z "$ramdisk" ] || [ -z "$system" ]; then
	echo "Usage: $0 <ramdisk> <system image>"
	exit 1
fi

workdir=`mktemp -d`
rootfs=$workdir/rootfs

mkdir -p $rootfs

# Extract ramdisk and preserve ownership of files
(cd $rootfs ; cat $ramdisk | gzip -d | sudo cpio -i)

sudo mkdir -p $rootfs/data/dalvik-cache/{arm,x86,x86_64}
sudo chown 0:0 $rootfs/data/dalvik-cache/{arm,x86,x86_64}

#sudo mkdir -p $rootfs/dev/pts
#sudo chown 0:0 $rootfs/dev/pts

sudo setfattr -n security.selinux -v "u:object_r:init_exec:s0" $rootfs/init
sudo setfattr -n security.selinux -v "u:object_r:init_exec:s0" $rootfs/anbox-init.sh
sudo setfattr -n security.selinux -v "u:object_r:rootfs:s0" $rootfs/init.environ.rc
sudo setfattr -n security.selinux -v "u:object_r:rootfs:s0" $rootfs/init.goldfish.rc
sudo setfattr -n security.selinux -v "u:object_r:rootfs:s0" $rootfs/init.rc
sudo setfattr -n security.selinux -v "u:object_r:rootfs:s0" $rootfs/init.usb.configfs.rc
sudo setfattr -n security.selinux -v "u:object_r:rootfs:s0" $rootfs/init.usb.rc
sudo setfattr -n security.selinux -v "u:object_r:rootfs:s0" $rootfs/init.zygote32.rc
sudo setfattr -n security.selinux -v "u:object_r:rootfs:s0" $rootfs/init.zygote64_32.rc
sudo setfattr -n security.selinux -v "u:object_r:system_file:s0" $rootfs/system
sudo setfattr -n security.selinux -v "u:object_r:system_data_file:s0" $rootfs/data
sudo setfattr -n security.selinux -v "u:object_r:dalvikcache_data_file:s0" $rootfs/data/dalvik-cache
sudo setfattr -n security.selinux -v "u:object_r:dalvikcache_data_file:s0" $rootfs/data/dalvik-cache/arm
sudo setfattr -n security.selinux -v "u:object_r:dalvikcache_data_file:s0" $rootfs/data/dalvik-cache/x86
sudo setfattr -n security.selinux -v "u:object_r:dalvikcache_data_file:s0" $rootfs/data/dalvik-cache/x86_64
sudo setfattr -n security.selinux -v "u:object_r:cache_file:s0" $rootfs/cache
sudo setfattr -n security.selinux -v "u:object_r:cgroup:s0" $rootfs/acct
sudo setfattr -n security.selinux -v "u:object_r:sysfs:s0" $rootfs/sys

mkdir $workdir/system
sudo mount -o loop,ro $system $workdir/system
sudo cp -ar $workdir/system/* $rootfs/system
echo "ro.sys.sdcardfs=0" | sudo tee -a $rootfs/system/build.prop
echo "ro.boot.fake_battery=1" | sudo tee -a $rootfs/system/build.prop

sudo umount $workdir/system

gcc -o $workdir/uidmapshift external/nsexec/uidmapshift.c
sudo $workdir/uidmapshift -b $rootfs 0 655360 65536
#sudo $workdir/uidmapshift -b $rootfs 0 100000 65536

sudo chown 655360:657360 $rootfs/init
sudo chown 655360:657360 $rootfs/init.environ.rc
sudo chown 655360:657360 $rootfs/init.goldfish.rc
sudo chown 655360:657360 $rootfs/init.rc
sudo chown 655360:657360 $rootfs/init.usb.configfs.rc
sudo chown 655360:657360 $rootfs/init.usb.rc
sudo chown 655360:657360 $rootfs/init.zygote32.rc
sudo chown 655360:657360 $rootfs/init.zygote64_32.rc
sudo chown 655360:657360 $rootfs/init

# sudo rmdir $rootfs/cache
# sudo ln -s /data/cache $rootfs/cache
sudo chown -h 655360:655360 $rootfs/cache
sudo chown 655360:656360 $rootfs/mnt
sudo chown 655360:656388 $rootfs/storage
sudo chown 656360:656360 $rootfs/data

sudo rm $rootfs/vendor
sudo mkdir $rootfs/vendor
sudo chown 655360:657360 $rootfs/vendor

sudo mkdir $rootfs/config/sdcardfs
sudo chown 656360:656360 $rootfs/config/sdcardfs

sudo mkdir -p $rootfs/var/run/arc
sudo chown -R 655360:655360 $rootfs/var

#sudo cp -a sdcard $rootfs/system/bin/sdcard
#sudo cp -a vold $rootfs/system/bin/vold

sudo bash $(dirname $0)/add-houdini.sh $rootfs
if [ $? != "0" ]; then echo "build houdini failed."; exit 1; fi

# FIXME
sudo chmod +x $rootfs/anbox-init.sh

sudo mksquashfs $rootfs android.img -comp xz -noappend
sudo chown $USER:$USER android.img

sudo rm -rf $workdir
