TEMPLATE = app

TARGET = SafeDisk

CONFIG += c++11

QT += widgets

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    Disk.cpp \
    CreateDiskDialog.cpp \
    App.cpp

RESOURCES += \
    resources.qrc

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    MainWindow.h \
    Disk.h \
    CreateDiskDialog.h \
    App.h

FORMS += \
    CreateDisk.ui

macx {
    OTHER_FILES += \
        Info.plist \
        scripts/osx/create_disk.sh \
        scripts/osx/mount_disk.sh \
        scripts/osx/unmount_disk.sh

    bundle_data.files = \
        $$OUT_PWD/../safediskd \
        scripts/osx/create_disk.sh \
        scripts/osx/mount_disk.sh \
        scripts/osx/unmount_disk.sh
    bundle_data.path = Contents/MacOS

    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
    QMAKE_INFO_PLIST = Info.plist
    QMAKE_BUNDLE_DATA += bundle_data
}
