HEADERS       = window.h \
                ObsWebSocket.h \
                ObsUtils.h \
                ObsAutoRecord.h
SOURCES       = main.cpp \
                window.cpp \
                ObsWebSocket.cpp \
                ObsUtils.cpp \
                ObsAutoRecord.cpp
RESOURCES     = obsautorecord.qrc

CONFIG += console

msvc: LIBS += -luser32 -lVersion

QT += websockets widgets
requires(qtConfig(combobox))
