#-------------------------------------------------
#
# Project created by QtCreator 2018-08-09T12:23:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++14
TARGET = qss
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        mainwindow.cpp \
       searchworker.cpp \
      pdfworker.cpp \
      pdfpreview.cpp \
      clicklabel.cpp

HEADERS += \
        mainwindow.h \
    searchworker.h \
    pdfworker.h \
    pdfpreview.h \
    clicklabel.h

FORMS += \
        mainwindow.ui

INCLUDEPATH += /usr/include/poppler/qt5/
LIBS += -lpoppler-qt5
QT += widgets sql

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../shared/release/ -lshared
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../shared/debug/ -lshared
else:unix: LIBS += -L$$OUT_PWD/../shared/ -lshared

INCLUDEPATH += $$PWD/../shared
DEPENDPATH += $$PWD/../shared

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../shared/release/libshared.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../shared/debug/libshared.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../shared/release/shared.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../shared/debug/shared.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../shared/libshared.a
