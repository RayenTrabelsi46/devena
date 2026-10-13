QT += core gui widgets printsupport charts network
CONFIG += c++17
TEMPLATE = app
TARGET = Devena

SOURCES += main.cpp participant.cpp
HEADERS += participant.h

# Lecture des CV au format PDF (Qt 6 avec le module pdf, sinon fichiers .txt seulement)
greaterThan(QT_MAJOR_VERSION, 5) {
    qtHaveModule(pdf) {
        QT += pdf
        DEFINES += HAS_QTPDF
    }
}