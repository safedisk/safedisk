TEMPLATE = app

QMAKE_MAC_SDK = macosx10.9
CONFIG += c++11

QT += widgets

SOURCES += \
    main.cpp \
    MainWindow.cpp

RESOURCES += \
    resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    MainWindow.h \
    MakeUnique.h
