TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += __USE_MINGW_ANSI_STDIO=1


HEADERS += parquet.h
HEADERS += thrift.h
HEADERS += misc.h

SOURCES += flecs.c
SOURCES += parquet.c
SOURCES += thrift.c
SOURCES += misc.c
SOURCES += main.c

LIBS += -lmingw32 -lws2_32

