QT       += core gui network multimedia sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Ipv6Client
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
        mainwindow.cpp \
    ownradioclient.cpp \
    ownradio.cpp

HEADERS  += mainwindow.h \
    ownradioclient.h \
    ownradio.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../qhttp/xbin/release/ -lqhttp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../qhttp/xbin/debug/ -lqhttp
else:unix: LIBS += -L$$PWD/../qhttp/xbin/ -lqhttp

INCLUDEPATH += $$PWD/../qhttp/xbin
INCLUDEPATH += $$PWD/../qhttp/src
DEPENDPATH += $$PWD/../qhttp/xbin

INCLUDEPATH += /usr/local/include/taglib

LIBS += -ltag
LIBS += -lz
