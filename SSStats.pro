QT       += core gui network
#CONFIG += debug
#CONFIG += qaxcontainer
CONFIG += static
QMAKE_CXXFLAGS += -std=c++11
TEMPLATE = app


TARGET = SSStats

PROJECT_PATH = "C:/projects/SSStatsCollector"
DESTDIR      = $$PROJECT_PATH
target.path = $$quote($$(PROGRAMFILES)/Steam/steamapps/common/Dawn of War Soulstorm)
target.files = $$DESTDIR/SSStats.exe
INSTALLS += target
RC_FILE = ssstats.rc
#INCLUDEPATH += F:/boost/include/boost-1_63


#PRE_TARGETDEPS += .beforebuild

#before_build.target = .beforebuild
#before_build.depends = FORCE
#before_build.commands = chcp 1251

#POST_TARGETDEPS += .afterbuild

#after_build.target = .afterbuild
#before_build.depends = FORCE
#before_build.commands = start $$quote($$(PROGRAMFILES)/Steam/steamapps/common/Dawn of War Soulstorm/SSStats.exe)

#TYPELIBS = TunngleSniffer.tlb
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
        types.h \
        OSDaB-Zip/unzip.h \
        OSDaB-Zip/unzip_p.h \
        OSDaB-Zip/zip.h \
        OSDaB-Zip/zip_p.h \
        OSDaB-Zip/zipentry_p.h \
        OSDaB-Zip/zipglobal.h \
        APMMeasure.h \
        version.h \
        defines.h \
        types.h

LIBS += -lpsapi -lz

#include(C:/projects/SSStatsCollector/qt_json/qt_json.pri)

unix|win32: LIBS += -L$$PWD/./ -lqxtglobalshortcut

INCLUDEPATH += $$PWD/qtxglobalshortcut
DEPENDPATH += $$PWD/qtxglobalshortcut

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/./qxtglobalshortcut.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/./libqxtglobalshortcut.a
