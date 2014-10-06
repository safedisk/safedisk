TEMPLATE = app

TARGET = SafeDisk

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
QMAKE_INFO_PLIST = Info.plist

CONFIG += c++11

QT += widgets

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    Disk.cpp \
    CreateDiskDialog.cpp

RESOURCES += \
    resources.qrc

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    MainWindow.h \
    Disk.h \
    CreateDiskDialog.h

OTHER_FILES += \
    Info.plist

FORMS += \
    CreateDisk.ui
