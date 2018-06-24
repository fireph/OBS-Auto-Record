HEADERS       = window.hpp \
                ObsWebSocket.hpp \
                ObsUtils.hpp \
                ObsAutoRecordState.hpp \
                ObsAutoRecord.hpp
SOURCES       = main.cpp \
                window.cpp \
                ObsWebSocket.cpp \
                ObsUtils.cpp \
                ObsAutoRecord.cpp
RESOURCES     = obsautorecord.qrc

release: DESTDIR = release/
debug:   DESTDIR = debug/

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

VERSION = 1.0.0.0
QMAKE_TARGET_COMPANY = "Frederick Meyer"
QMAKE_TARGET_PRODUCT = "OBS Auto Record"
QMAKE_TARGET_DESCRIPTION = "OBS Auto Record"
QMAKE_TARGET_COPYRIGHT = "\251 2018 Frederick Meyer"

#CONFIG += console

msvc: LIBS += -luser32 -lVersion

win32: RC_ICONS += images/record_red.ico

QT += websockets widgets
requires(qtConfig(combobox))
