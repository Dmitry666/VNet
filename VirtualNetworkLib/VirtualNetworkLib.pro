#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T23:33:20
#
#-------------------------------------------------

TARGET = VirtualNetworkLib
TEMPLATE = lib

CONFIG += c++11

DEFINES += WITH_SSL __COMPILING_VNET

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


win32 {
    # Boost
    INCLUDEPATH += $(EXTERNALDIR)/boost_1_59_0

    # OpenSLL
    INCLUDEPATH += $(EXTERNALDIR)/openssl-1.0.1c/include
}

win32 {
    BOOST_VER = 1_59
    COMPILER_SHORT = mgw48

    debug {
    BOOST_SUFFIX = $${COMPILER_SHORT}-sd-$${BOOST_VER}
    }

    release {
    BOOST_SUFFIX = $${COMPILER_SHORT}-s-$${BOOST_VER}
    }

    LIBS += -L$(EXTERNALDIR)/boost_1_59_0/stage/lib

    LIBS += -lboost_system-$${BOOST_SUFFIX} -lboost_filesystem-$${BOOST_SUFFIX}

    debug {
    LIBS += -lboost_thread-mgw48-mt-sd-1_59
    }

    release {
    LIBS += -lboost_thread-mgw48-mt-s-1_59
    }

    LIBS += -lws2_32 -lmswsock
}

unix {
    LIBS += -L/opt/local/lib
    LIBS += -L/usr/local/lib
    LIBS += -Wl,-rpath=/usr/local/lib
    LIBS += -lboost_system -lboost_filesystem -lboost_thread -lboost_serialization
    LIBS += -lpthread


    LIBS += -lssl -lcrypto
}

LIBS += -lVirtualNetworkCommon
