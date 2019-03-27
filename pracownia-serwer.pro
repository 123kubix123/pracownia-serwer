#-------------------------------------------------
#
# Project created by QtCreator 2016-12-28T17:33:17
#
#-------------------------------------------------

QT       += core gui network sql webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pracownia-serwer
TEMPLATE = app


SOURCES += main.cpp\
        serwer.cpp \
    extended_connection.cpp \
    panel_browser.cpp

HEADERS  += serwer.h \
    extended_connection.h \
    panel_browser.h

FORMS    += serwer.ui \
    panel_browser.ui

RESOURCES += \
    resources.qrc
