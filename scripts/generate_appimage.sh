#!/bin/bash

rm -rf AppDir
mkdir Temp
mkdir Output

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
mv linuxdeploy-x86_64.AppImage Temp/linuxdeploy-x86_64.AppImage
wget https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
mv appimagetool-x86_64.AppImage Temp/appimagetool-x86_64.AppImage

chmod 0744 Temp/linuxdeploy-x86_64.AppImage
chmod 0744 Temp/appimagetool-x86_64.AppImage

NO_STRIP=true ./Temp/linuxdeploy-x86_64.AppImage --appdir AppDir --executable ./../build/src/gxt2edit -d ./gxt2edit.desktop -i ./icon_gxt2edit.png

cp -r ../fonts AppDir/

NO_STRIP=true ARCH=x86_64 ./Temp/appimagetool-x86_64.AppImage AppDir Output/gxt2edit.AppImage
