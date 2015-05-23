
# put common definitions in here
#PROFILE = debug
PROFILE = release

PREFIX_RELEASE = /usr/local
#PREFIX_RELEASE = ${HOME}
PREFIX_DEBUG = ${HOME}

# useful defines that help with debugging when things get wierd:
# POE_DBG_TRON - turns on function tracing, mildly reducing performance (~ 5%)
# POE_DBG_MARKPTRS - turns on mark validity checking
# POE_DBG_BUFPTRS - turns on buffer validity checking
# POE_DBG_LIM - turns on limit checking in vector and string types

PRJCFLAGS_RELEASE = -Os -Wall -DPOE_DBG_TRON -DPOE_DBG_MARKPTRS -DPOE_DBG_BUFPTRS -DPOE_DBG_LIM
#PRJCFLAGS_RELEASE = -Os -Wall

#PRJCFLAGS_DEBUG = -g -O0 -Wall -DPOE_DBG_TRON -DPOE_DBG_MARKPTRS -DPOE_DBG_BUFPTRS -DPOE_DBG_LIM
PRJCFLAGS_DEBUG = -g -O0 -Wall

LDFLAGS_RELEASE = -flto
LDFLAGS_DEBUG =

ARFLAGS_RELEASE =
ARFLAGS_DEBUG =


CC      = cc
LD      = cc
AR      = ar
RANLIB  = ranlib
RM      = rm
ECHO = echo
SHELL   = /bin/sh


.if "${PROFILE}" == "release"
PREFIX = $(PREFIX_RELEASE)
PRJCFLAGS = $(PRJCFLAGS_RELEASE)
LDFLAGS = $(LDFLAGS_RELEASE)
ARFLAGS = ${ARFLAGS_RELEASE}
STRIP = strip
.elif "${PROFILE}" == "debug"
PREFIX = $(PREFIX_DEBUG)
PRJCFLAGS = $(PRJCFLAGS_DEBUG)
LDFLAGS = $(LDFLAGS_DEBUG)
ARFLAGS = ${ARFLAGS_DEBUG}
STRIP = false
.endif
