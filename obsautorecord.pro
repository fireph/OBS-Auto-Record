HEADERS       = window.h \
                ObsWebSocket.h \
                ObsAutoRecord.h
SOURCES       = main.cpp \
                window.cpp \
                ObsWebSocket.cpp \
                ObsAutoRecord.cpp
RESOURCES     = obsautorecord.qrc

CONFIG += console

QT += websockets widgets
requires(qtConfig(combobox))
