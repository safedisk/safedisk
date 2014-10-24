TEMPLATE = app

TARGET = SafeDisk

CONFIG += c++11

QT += widgets

qtHaveModule(svg) {
    QT += svg
}

SOURCES += \
    MainWindow.cpp \
    Disk.cpp \
    CreateDiskDialog.cpp \
    App.cpp \
    DiskWidget.cpp \
    Script.cpp

RESOURCES += \
    resources.qrc

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    MainWindow.h \
    Disk.h \
    CreateDiskDialog.h \
    App.h \
    DiskWidget.h \
    Script.h

FORMS += \
    CreateDisk.ui \
    About.ui

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

test {
    message(Building tests)

    TARGET = unittest
    QT += testlib

    CONFIG += console
    CONFIG -= app_bundle

    SOURCES += \
        Disk_test.cpp
}
else {
    message(Building app)

    SOURCES += \
        main.cpp
}
