QT += core gui widgets sql printsupport

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    projet.cpp \
    employe.cpp \
    connection.cpp

HEADERS += \
    mainwindow.h \
    projet.h \
    employe.h \
    connection.h

FORMS += \
    employe.ui

RESOURCES += \
    resources.qrc