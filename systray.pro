HEADERS       = window.h
SOURCES       = main.cpp \
                window.cpp
RESOURCES     = systray.qrc

QT += widgets
requires(qtConfig(combobox))

# install
target.path = /Users/fmeyer/Downloads/systray
INSTALLS += target