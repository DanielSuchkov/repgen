#-------------------------------------------------
#
# Project created by QtCreator 2015-08-12T00:02:51
#
#-------------------------------------------------

QT       -= gui

TARGET = repgen
CONFIG   += console \
            c++11
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

unix {
    INCLUDEPATH += /home/fallen/data/programing/c++/lib/boost
    LIBPATH = /home/fallen/data/programing/c++/lib/boost/stage/lib/
    LIBS += $${LIBPATH}libboost_filesystem.a \
        $${LIBPATH}libboost_system.a
}

HEADERS += \
    Generator.h
