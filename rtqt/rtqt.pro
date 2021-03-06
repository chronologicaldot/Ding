######################################################################
# Automatically generated by qmake (3.0) Thu Jan 31 16:13:07 2019
######################################################################

TEMPLATE = app
TARGET = rtqt
INCLUDEPATH += .
INCLUDEPATH += ../rtaudio
INCLUDEPATH += ../Ding/src/soundbox
LIBS += -lasound
LIBS += -lpthread

CONFIG += c++11
QT += widgets
DEFINES += "__LINUX_ALSA__" # For RtAudio. Should check for Linux first.

# Input
HEADERS += app.h \
           ../rtaudio/RtAudio.h \
           ../Ding/src/soundbox/Soundbox.h
SOURCES += main.cpp app.cpp \
           ../rtaudio/RtAudio.cpp \
           ../Ding/src/soundbox/SoundBox.cpp
