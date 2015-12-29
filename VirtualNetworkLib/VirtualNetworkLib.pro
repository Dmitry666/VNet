#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T23:33:20
#
#-------------------------------------------------

TARGET = VirtualNetworkLib
TEMPLATE = lib

CONFIG += c++11

LIBS += -lVirtualNetworkCommon

HEADERS += \
    Client.h \
    Common.h \
    CommonPrivate.h \
    Delegate.h \
    DirectSocket.h \
    Listerner.h \
    Socket.h \
    TcpClient.h \
    TcpConnection.h \
    TcpConnectionManager.h \
    TcpServer.h \
    VirtualNetwork.h \
    VirtualNetworkPrivate.h \
    VirtualSocket.h \
    VN.h

SOURCES += \
    Client.cpp \
    DirectSocket.cpp \
    Listerner.cpp \
    Socket.cpp \
    TcpClient.cpp \
    TcpConnection.cpp \
    TcpConnectionManager.cpp \
    TcpServer.cpp \
    VirtualNetwork.cpp \
    VirtualNetworkPrivate.cpp \
    VirtualSocket.cpp


