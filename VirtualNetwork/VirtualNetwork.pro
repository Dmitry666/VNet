#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T23:33:20
#
#-------------------------------------------------

TARGET = VirtualNetwork
TEMPLATE = app

CONFIG += c++11
DEFINES += WITH_SSL

HEADERS += \
    Common.h \
    CommonController.h \
    SessionManager.h \
    UserController.h \
    tcp/tcp_connection.hpp \
    tcp/tcp_connection_manager.hpp \
    tcp/tcp_reply.hpp \
    tcp/tcp_request.hpp \
    tcp/tcp_request_handler.hpp \
    tcp/tcp_request_parser.hpp \
    tcp/tcp_server.hpp

SOURCES += \
    CommonController.cpp \
    main.cpp \
    SessionManager.cpp \
    UserController.cpp \
    tcp/tcp_connection.cpp \
    tcp/tcp_connection_manager.cpp \
    tcp/tcp_reply.cpp \
    tcp/tcp_request_handler.cpp \
    tcp/tcp_request_parser.cpp \
    tcp/tcp_server.cpp

win32 {
    # Boost
    INCLUDEPATH += $(EXTERNALDIR)/boost_1_59_0

    # OpenSLL
    INCLUDEPATH += $(EXTERNALDIR)/openssl-1.0.1c/include

    # OpenSLL
    INCLUDEPATH += $(EXTERNALDIR)/sqlite3
}

LIBS += -L../VirtualNetworkCommon

LIBS += -lVirtualNetworkCommon
#LIBS += -lHTTPCommandService -ldl -ljsoncpp



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
    LIBS += -lboost_system -lboost_filesystem -lboost_thread
    LIBS += -lpthread


    LIBS += -lssl -lcrypto
    LIBS += -lsqlite3
}
