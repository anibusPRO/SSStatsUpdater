#-------------------------------------------------
#
# Project created by QtCreator 2016-09-23T16:34:40
#
#-------------------------------------------------

QT       += core network
QT       -= gui

QMAKE_CXXFLAGS += -std=c++11

CONFIG -= console

TEMPLATE = app
#TEMPLATE = lib
#CONFIG += static
#DEFINES += BOOST_NO_EXCEPTIONS

TARGET = SSStats

PROJECT_PATH = "C:/OpenServer/domains/dowstats/ssstats"
DESTDIR      = $$PROJECT_PATH

#static { # everything below takes effect with CONFIG ''= static
# CONFIG += static
# CONFIG += staticlib # this is needed if you create a static library, not a static executable
# DEFINES+= STATIC
# message("~~~ static build ~~~") # this is for information, that the static build is done
# mac: TARGET = $$join(TARGET,,,_static) #this adds an _static in the end, so you can seperate static build from non static build
# win32: TARGET = $$join(TARGET,,,s) #this adds an s in the end, so you can seperate static build from non static build
#}

#CONFIG (debug, debug|release) {
# mac: TARGET = $$join(TARGET,,,_debug)
# win32: TARGET = $$join(TARGET,,,d)
#}

INCLUDEPATH += C:/boost/include/boost-1_63

DEFINES += SSSTATS_LIBRARY

SOURCES += main.cpp \
        request.cpp \
        requestsender.cpp \
        gameinforeader.cpp \
        gameinfo.cpp \
        statscollector.cpp \
        logger.cpp \
        replay.cpp \
        game_action.cpp \
        player.cpp \
        winconditions.cpp \
        gamesettings.cpp \
        repreader.cpp \
        extendedbinreader.cpp \
        apmmeter.cpp \
        systemwin32.cpp \
    OSDaB-Zip/unzip.cpp \
    OSDaB-Zip/zip.cpp \
    OSDaB-Zip/zipglobal.cpp


HEADERS += request.h \
        requestsender.h \
        gameinforeader.h \
        gameinfo.h \
        statscollector.h \
        logger.h \
        replay.h \
        game_action.h \
        player.h \
        winconditions.h \
        gamesettings.h \
        repreader.h \
        extendedbinreader.h \
        apmmeter.h \
        systemwin32.h \
    vdf_parser.hpp \
    OSDaB-Zip/unzip.h \
    OSDaB-Zip/unzip_p.h \
    OSDaB-Zip/zip.h \
    OSDaB-Zip/zip_p.h \
    OSDaB-Zip/zipentry_p.h \
    OSDaB-Zip/zipglobal.h


SOURCES += APMShared/APMConfig.cpp \
    APMShared/APMLogger.cpp \
    APMShared/APMMeasure.cpp \
    APMShared/APMTrigger.cpp \
    APMShared/ProcessResolver.cpp

HEADERS +=    APMShared/APMConfig.h \
    APMShared/APMLogger.h \
    APMShared/APMMeasure.h \
    APMShared/APMTrigger.h \
    APMShared/ProcessResolver.h

include(qt_json/qt_json.pri)
#!contains(DEFINES, HAVE_QT5)
