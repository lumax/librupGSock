PROJECT_NAME=librupGSock
MAJOR=1
MINOR=0
VERSION=$(MAJOR).$(MINOR)

DEFS+=-D_GNU_SOURCE=1 -D_REENTRANT
INCLUDES+=-I$(INCLUDE_DIR)/libruputils
LIBS+=-L$(LIB_DIR)

CFLAGS = `pkg-config --cflags glib-2.0`
LDFLAGS = `pkg-config --libs glib-2.0` -lruputils

CFLAGS+=-g -c -Wall -fPIC
LDFLAGS+=-shared -Wl

OBJS = rupGSock.o

EXE_ANHANG = .so.$(VERSION)

include $(MAKE_DIR)/global.mak

