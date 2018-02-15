TARGETS=all check clean clobber distclean install uninstall
TARGET=all

PREFIX=${DESTDIR}/opt
BINDIR=${PREFIX}/bin
SUBDIRS=

ifeq	(${MAKE},gmake)
	INSTALL=ginstall
else
	INSTALL=install
endif

.PHONY: ${TARGETS} ${SUBDIRS}

all::	autoauth.so

${TARGETS}::

clobber distclean:: clean

check::	autoauth.so
	./autoauth.so ${ARGS}

install:: autoauth.so
	${INSTALL} -D autoauth.so ${BINDIR}/autoauth.so

uninstall::
	${RM} ${BINDIR}/autoauth.so

CFLAGS	=-O3 -fPIC
CFLAGS	+=`pkg-config --cflags purple`
CFLAGS	+=`pkg-config --cflags glib-2.0`

LDFLAGS	= -shared `pkg-config --libs glib-2.0`
LDFLAGS	+=-shared `pkg-config --libs purple`

autoauth.so:: autoauth.c
	gcc -o $@ ${CFLAGS} ${LDFLAGS} $<

ifneq	(,${SUBDIRS})
${TARGETS}::
	${MAKE} TARGET=$@ ${SUBDIRS}
${SUBDIRS}::
	${MAKE} -C $@ ${TARGET}
endif
