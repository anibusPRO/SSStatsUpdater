QT       += core gui network
CONFIG -= console
CONFIG += debug

QMAKE_CXXFLAGS += -std=c++11
TEMPLATE = app


TARGET = SSStats

PROJECT_PATH = "C:/OpenServer/domains/dowstats.loc/ssstats"
DESTDIR      = $$PROJECT_PATH
target.path = "C:/Program Files (x86)/Steam/steamapps/common/Dawn of War Soulstorm"
target.files = $$DESTDIR/SSStats.exe
INSTALLS += target
RC_FILE = ssstats.rc
INCLUDEPATH += C:/boost/include/boost-1_63

SOURCES += main.cpp \
        request.cpp \
        requestsender.cpp \
        gameinforeader.cpp \
        gameinfo.cpp \
        statscollector.cpp \
        logger.cpp \
        replay.cpp \
        player.cpp \
        gamesettings.cpp \
        repreader.cpp \
        extendedbinreader.cpp \
        apmmeter.cpp \
        systemwin32.cpp \
        OSDaB-Zip/unzip.cpp \
        OSDaB-Zip/zip.cpp \
        OSDaB-Zip/zipglobal.cpp \
        APMMeasure.cpp \
    qtxglobalshortcut/qxtglobalshortcut.cpp \
    qtxglobalshortcut/qxtglobalshortcut_win.cpp


HEADERS += request.h \
        requestsender.h \
        gameinforeader.h \
        gameinfo.h \
        statscollector.h \
        logger.h \
        replay.h \
        game_action.h \
        player.h \
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
        OSDaB-Zip/zipglobal.h \
        APMMeasure.h \
        version.h \
        defines.h \
    qtxglobalshortcut/qxtglobal.h \
    qtxglobalshortcut/qxtglobalshortcut.h \
    qtxglobalshortcut/qxtglobalshortcut_p.h


LIBS += -lpsapi

include(C:/Programming/SSStats/qt_json/qt_json.pri)

#win32 {
#    WINSDK_DIR = C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A
#    QMAKE_POST_LINK = "\"$$WINSDK_DIR/bin/x64/mt.exe\" -manifest \"$$PWD/$$basename(TARGET).manifest\" -outputresource:\"$$OUT_PWD/${DESTDIR_TARGET};1\""
#}
