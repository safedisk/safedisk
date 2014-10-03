TEMPLATE = app

QMAKE_MAC_SDK = macosx10.9
CONFIG += c++11

QT += qml quick widgets

SOURCES += main.cpp

RESOURCES += qml.qrc \
    resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
