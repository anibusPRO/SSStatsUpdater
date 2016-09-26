#-------------------------------------------------
#
# Project created by QtCreator 2016-09-23T16:34:40
#
#-------------------------------------------------

QT       += core network
QT       -= gui
QMAKE_CXXFLAGS += -std=c++11
CONFIG -= console

TARGET = SSStats
TEMPLATE = lib

DEFINES += SSSTATS_LIBRARY

SOURCES +=     main.cpp \
    request.cpp \
    requestsender.cpp \
    gameinforeader.cpp \
    gameinfo.cpp \
    statscollector.cpp \
    logger.cpp
#ssstats.cpp \


HEADERS += request.h \
        requestsender.h \
        gameinforeader.h \
        gameinfo.h \
        statscollector.h \
    logger.h
#ssstats.h\
#        ssstats_global.h \





#TARGET = soulstorm_stats
#TEMPLATE = app


#SOURCES +=

#HEADERS += \


!contains(DEFINES, HAVE_QT5)    include(qt_json/qt_json.pri)
