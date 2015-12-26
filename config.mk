# dvtm version
VERSION = 0.14ms

# Customize below to fit your system

PREFIX ?= /usr/local
MANPREFIX = ${PREFIX}/share/man
# specify your systems terminfo directory
# leave empty to install into your home folder
TERMINFO := ${DESTDIR}${PREFIX}/share/terminfo

INCS = -I.
LIBS = -lc -lutil -lncursesw -ldialog -lboost_system -lboost_filesystem -ldl -lpthread
CPPFLAGS = -std=gnu++11 -Wall -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_XOPEN_SOURCE_EXTENDED
CFLAGS += -std=c99  ${INCS} -DVERSION=\"${VERSION}\" -DNDEBUG -DHAVE_NCURSESW -Wall -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_XOPEN_SOURCE_EXTENDED
LDFLAGS += ${LIBS}

DEBUG_CFLAGS = ${CFLAGS} -UNDEBUG -O0 -g -ggdb -Wall -Wextra -Wno-unused-parameter

CC ?= cc
CXX ?= g++
STRIP ?= strip
