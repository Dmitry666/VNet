#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T23:33:20
#
#-------------------------------------------------

TARGET = VirtualNetwork
TEMPLATE = app

CONFIG += c++11

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

LIBS += -lHTTPCommandService -ldl -ljsoncpp

