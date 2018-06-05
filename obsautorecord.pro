HEADERS       = window.h
SOURCES       = main.cpp \
                window.cpp
RESOURCES     = obsautorecord.qrc

QT += websockets widgets
requires(qtConfig(combobox))
