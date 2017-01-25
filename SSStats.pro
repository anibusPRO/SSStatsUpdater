#-------------------------------------------------
#
# Project created by QtCreator 2016-09-23T16:34:40
#
#-------------------------------------------------

QT       += core network
QT       -= gui

QMAKE_CXXFLAGS += -std=c++11

CONFIG -= console

TEMPLATE = lib
CONFIG += dll


TARGET = SSStats


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


DEFINES += SSSTATS_LIBRARY

SOURCES +=     main.cpp \
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
#    languageservice.cpp \
    apmmeter.cpp
#ssstats.cpp \


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
#    languageservice.h \
    apmmeter.h
#ssstats.h\
#        ssstats_global.h \


SOURCES += APMShared/APMConfig.cpp \
    APMShared/APMLogger.cpp \
    APMShared/APMMeasure.cpp \
    APMShared/APMTrigger.cpp \
    APMShared/ProcessResolver.cpp

#HEADERS += APMKeyHook/targetver.h \
HEADERS +=    APMShared/APMConfig.h \
    APMShared/APMLogger.h \
    APMShared/APMMeasure.h \
    APMShared/APMTrigger.h \
    APMShared/ProcessResolver.h


#TARGET = soulstorm_stats
#TEMPLATE = app


#SOURCES +=

#HEADERS += \
INCLUDEPATH += C:/Qt/qt-static/include

LIBS        +=        -L"c:/Qt/qt-static/lib" -lQtNetwork -lQtCore -lole32 -luuid -lws2_32 -ladvapi32 -lshell32 -luser32 -lkernel32 -lz


include(qt_json/qt_json.pri)
#!contains(DEFINES, HAVE_QT5)
