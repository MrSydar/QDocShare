QT       += core gui
QT       += concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    clientrunner.cpp \
    communication.cpp \
    document.cpp \
    errordialog.cpp \
    nettexteditor.cpp \
    quitInfo.cpp \
    serverrunner.cpp \
    clientwindow.cpp \
    mainwindow.cpp \
    main.cpp

HEADERS += \
    clientrunner.h \
    errordialog.h \
    nettexteditor.h \
    serverrunner.h \
    clientwindow.h \
    mainwindow.h \
    communication.h \
    document.h \
    quitInfo.h

FORMS += \
    clientwindow.ui \
    errordialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=
