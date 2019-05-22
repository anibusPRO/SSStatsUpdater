#-------------------------------------------------
#
# Project created by QtCreator 2016-12-28T20:04:52
#
#-------------------------------------------------

QT       += core network xml
#QT       -= gui
CONFIG -= console
QMAKE_CXXFLAGS += -std=c++11

#TEMPLATE = lib
TEMPLATE = app

#CONFIG += dll qt

TARGET = SSStatsUpdater

RC_FILE = ssstatsupdater.rc

DEFINES += SSSTATSUPDATER_LIBRARY

PROJECT_PATH = "F:/OpenServer/domains/dowstats.loc/ssstats"
DESTDIR      = $$PROJECT_PATH
target.path = $$quote($$(PROGRAMFILES)/Steam/steamapps/common/Dawn of War Soulstorm)
target.files = $$DESTDIR/SSStatsUpdater.exe
INSTALLS += target
INCLUDEPATH += F:/boost/include/boost-1_63

SOURCES += \
    main.cpp \
    statsupdater.cpp \
    ../SSStats/systemwin32.cpp \
    ../SSStats/request.cpp \
    ../SSStats/requestsender.cpp \
    ../SSStats/OSDaB-Zip/unzip.cpp \
    ../SSStats/OSDaB-Zip/zip.cpp \
    ../SSStats/OSDaB-Zip/zipglobal.cpp \
    ../SSStats/logger.cpp
#    form.cpp

HEADERS +=\
    statsupdater.h \
    ../SSStats/systemwin32.h \
    ../SSStats/request.h \
    ../SSStats/requestsender.h \
    ../SSStats/OSDaB-Zip/unzip.h \
    ../SSStats/OSDaB-Zip/unzip_p.h \
    ../SSStats/OSDaB-Zip/zip.h \
    ../SSStats/OSDaB-Zip/zip_p.h \
    ../SSStats/OSDaB-Zip/zipentry_p.h \
    ../SSStats/OSDaB-Zip/zipglobal.h \
    ../SSStats/logger.h \
    version.h
#    form.h

#FORMS += form.ui

RESOURCES += ssstatsuptater.qrc

win32 {
    WINSDK_DIR = F:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A
#    QMAKE_POST_LINK = "\"$$WINSDK_DIR/bin/mt.exe\" -manifest \"$$PWD/$$basename(TARGET).manifest\" -outputresource:\"$$OUT_PWD/${DESTDIR_TARGET};1\""
    QMAKE_POST_LINK = "\"$$WINSDK_DIR/bin/mt.exe\" -manifest \"$$PWD/$$basename(TARGET).manifest\" -outputresource:\"${DESTDIR_TARGET};1\""
}



