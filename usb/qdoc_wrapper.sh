#!/bin/sh
QT_VERSION=0.7.0
export QT_VERSION
QT_VER=0.7
export QT_VER
QT_VERSION_TAG=070
export QT_VERSION_TAG
QT_INSTALL_DOCS=C:/Qt/Qt5.12.9/Docs/Qt-5.12.9
export QT_INSTALL_DOCS
BUILDDIR=D:/GitRepositories/QtUsb/src/usb
export BUILDDIR
exec 'C:\Qt\Qt5.12.9\5.12.9\mingw73_32\bin\qdoc.exe' "$@"
