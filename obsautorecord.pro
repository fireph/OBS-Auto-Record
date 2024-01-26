HEADERS   = ObsAutoRecord.hpp \
            ObsAutoRecordState.hpp \
            ObsSettingsDialog.hpp \
            ObsUtils.hpp \
            ObsWebSocket.hpp
SOURCES   = main.cpp \
            ObsAutoRecord.cpp \
            ObsSettingsDialog.cpp \
            ObsUtils.cpp \
            ObsWebSocket.cpp
RESOURCES = obsautorecord.qrc

macx {
  HEADERS += ObsUtilsOSX.hpp
  OBJECTIVE_SOURCES += ObsUtilsOSX.mm
  TARGET = "OBS Auto Record"
}

CONFIG(debug, debug|release) {
    DESTDIR = debug/
} else {
    DESTDIR = release/
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

VERSION = 2.0.5
QMAKE_TARGET_COMPANY = "Frederick Meyer"
QMAKE_TARGET_PRODUCT = "OBS Auto Record"
QMAKE_TARGET_DESCRIPTION = "OBS Auto Record"
QMAKE_TARGET_COPYRIGHT = "\251 2019 Frederick Meyer"

#CONFIG += console

msvc: LIBS += -luser32 -lVersion

win32: RC_ICONS += images/record_red.ico
macx: ICON = images/record_red.icns

macx: LIBS += -framework AppKit

QT += websockets widgets
requires(qtConfig(combobox))

win32: include($$PWD\QHotkey\qhotkey.pri)
macx: include($$PWD/QHotkey/qhotkey.pri)
DEPENDPATH  += $$PWD/QHotkey/QHotkey
INCLUDEPATH += $$PWD/QHotkey/QHotkey
