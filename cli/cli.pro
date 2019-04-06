QT -= gui
QT += sql concurrent
CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS += -luchardet -lpoppler-qt5 -lquazip
SOURCES += \
        main.cpp \
    encodingdetector.cpp \
    processor.cpp \
    pdfprocessor.cpp \
    defaulttextprocessor.cpp \
    command.cpp \
    commandadd.cpp \
    tagstripperprocessor.cpp \
    nothingprocessor.cpp \
    odtprocessor.cpp \
    utils.cpp \
    odsprocessor.cpp \
    qssgeneralexception.cpp

HEADERS += \
    encodingdetector.h \
    processor.h \
    pagedata.h \
    pdfprocessor.h \
    defaulttextprocessor.h \
    command.h \
    commandadd.h \
    tagstripperprocessor.h \
    nothingprocessor.h \
    odtprocessor.h \
    utils.h \
    odsprocessor.h \
    qssgeneralexception.h
INCLUDEPATH += /usr/include/poppler/qt5/ /usr/include/quazip5
