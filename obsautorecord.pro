HEADERS       = window.h \
                echoclient.h
SOURCES       = main.cpp \
                window.cpp \
                echoclient.cpp
RESOURCES     = obsautorecord.qrc

QT += websockets widgets
requires(qtConfig(combobox))
