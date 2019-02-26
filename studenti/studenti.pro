#-------------------------------------------------
#
# Project created by QtCreator 2017-01-13T16:17:19
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

CONFIG += c++11

TARGET = studenti
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    predmet.cpp

HEADERS  += mainwindow.h \
    predmet.h

DISTFILES += \
    create-db.sql

RESOURCES += \
    text.qrc
