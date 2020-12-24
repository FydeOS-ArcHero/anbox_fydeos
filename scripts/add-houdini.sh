#!/system/bin/sh

rootfs=$1

if [ -f "$rootfs/system/lib/arm/libhoudini.so $rootfs/system/lib/libhoudini.so" ]; then
  exit
fi
     
wget https://github.com/redchenjs/aur-packages/releases/download/anbox-image/houdini_y.sfs -O /tmp/houdini_y.sfs
if [ $? != "0" ]; then exit 1; fi
unsquashfs -f -d ./houdini_y /tmp/houdini_y.sfs

mkdir -p $rootfs/system/lib/arm
cp -r ./houdini_y/* $rootfs/system/lib/arm
mv $rootfs/system/lib/arm/libhoudini.so $rootfs/system/lib/libhoudini.so

wget https://github.com/redchenjs/aur-packages/releases/download/anbox-image/houdini_z.sfs -O /tmp/houdini_z.sfs
if [ $? != "0" ]; then exit 1; fi
unsquashfs -f -d ./houdini_z /tmp/houdini_z.sfs

mkdir -p $rootfs/system/lib64/arm64
cp -r ./houdini_z/* $rootfs/system/lib64/arm64
mv $rootfs/system/lib64/arm64/libhoudini.so $rootfs/system/lib64/libhoudini.so

# add houdini parser
mkdir -p $rootfs/system/etc/binfmt_misc
echo ':arm_exe:M::\x7f\x45\x4c\x46\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x28::/system/lib/arm/houdini:P' >> $rootfs/system/etc/binfmt_misc/arm_exe
echo ':arm_dyn:M::\x7f\x45\x4c\x46\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x28::/system/lib/arm/houdini:P' >> $rootfs/system/etc/binfmt_misc/arm_dyn
echo ':arm64_exe:M::\x7f\x45\x4c\x46\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\xb7::/system/lib64/arm64/houdini64:P' >> $rootfs/system/etc/binfmt_misc/arm64_exe
echo ':arm64_dyn:M::\x7f\x45\x4c\x46\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\xb7::/system/lib64/arm64/houdini64:P' >> $rootfs/system/etc/binfmt_misc/arm64_dyn


# add features
sed -i '/<\/permissions>/d' $rootfs/system/etc/permissions/anbox.xml
sed -i '/<unavailable-feature name=\"android.hardware.wifi\" \/>/d' $rootfs/system/etc/permissions/anbox.xml
sed -i '/<unavailable-feature name=\"android.hardware.bluetooth\" \/>/d' $rootfs/system/etc/permissions/anbox.xml

echo '    <feature name="android.hardware.touchscreen" />
<feature name="android.hardware.audio.output" />
<feature name="android.hardware.camera" />
<feature name="android.hardware.camera.any" />
<feature name="android.hardware.location" />
<feature name="android.hardware.location.gps" />
<feature name="android.hardware.location.network" />
<feature name="android.hardware.microphone" />
<feature name="android.hardware.screen.portrait" />
<feature name="android.hardware.screen.landscape" />
<feature name="android.hardware.wifi" />
<feature name="android.hardware.bluetooth" />' >> $rootfs/system/etc/permissions/anbox.xml
echo '</permissions>' >> $rootfs/system/etc/permissions/anbox.xml

# set processors
sed -i '/^ro.product.cpu.abilist=x86_64,x86/ s/$/,arm64-v8a,armeabi-v7a,armeabi/' $rootfs/system/build.prop
sed -i '/^ro.product.cpu.abilist32=x86/ s/$/,armeabi-v7a,armeabi/' $rootfs/system/build.prop
sed -i '/^ro.product.cpu.abilist64=x86_64/ s/$/,arm64-v8a/' $rootfs/system/build.prop

# enable nativebridge
echo 'persist.sys.nativebridge=1' >> $rootfs/system/build.prop
sed -i 's/ro.dalvik.vm.native.bridge=0/ro.dalvik.vm.native.bridge=libhoudini.so/' $rootfs/default.prop

# enable opengles
echo 'ro.opengles.version=131072' >> $rootfs/system/build.prop

# install media codecs
cp $(dirname $0)/houdini/media_codec*.xml $rootfs/system/etc/

rm -rf ./houdini_z/ ./houdini_y/