HEADERS       = window.h \
                ObsWebSocket.h
SOURCES       = main.cpp \
                window.cpp \
                ObsWebSocket.cpp
RESOURCES     = obsautorecord.qrc

CONFIG += console

QT += websockets widgets
requires(qtConfig(combobox))
