#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T23:33:20
#
#-------------------------------------------------

TARGET = VirtualNetworkTestClient
TEMPLATE = app

CONFIG += c++11

LIBS += -lVirtualNetworkLib

SOURCES += \
    main.cpp

win32 {
    # Boost
    INCLUDEPATH += $(EXTERNALDIR)/boost_1_59_0

    # OpenSLL
    INCLUDEPATH += $(EXTERNALDIR)/openssl-1.0.1c/include
}

LIBS += -lVirtualNetworkLib
