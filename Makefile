include config.mk

SRC = dvtm.c vt.c
CXX_SRC=script.cc init.cc
OBJ = ${SRC:.c=.o} ${CXX_SRC:.cc=.o}

all: options dvtm

options:
	@echo dvtm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CPPFLAGS = ${CPPFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

config.h:
	cp config.def.h config.h

init.cc: init.chai
	@echo "CHAI2CC init.chai"
	@echo -n "const char *dvtm_script_initialization_string=\"" >init.cc
	@cat init.chai | hexdump -v -e '/1 " %02X"' | sed 's/ /\\x/g' >>init.cc
	@echo "\";" >>init.cc
    
script.cc: init.cc

%.o:%.c $(DEPDIR)/%.d
	@echo CC $<
	@${CC} -c ${CFLAGS} $<
	@$(POSTCOMPILE)
	
%.o: %.cc $(DEPDIR)/%.d
	@echo CXX $<
	@${CXX} -c ${CPPFLAGS} $<
	@$(POSTCOMPILE)

precompiled_headers.h.gch: precompiled_headers.h
	@echo CXX precompiled_headers.h
	@${CXX} -c ${CPPFLAGS} precompiled_headers.h -o precompiled_headers.h.gch
	

${OBJ}: config.h config.mk precompiled_headers.h.gch

dvtm: ${OBJ}
	@echo CXX -o $@
	@${CXX} -o $@ ${OBJ} ${LDFLAGS}

debug: clean
	@make CFLAGS='${DEBUG_CFLAGS}'

clean:
	@echo cleaning
	@rm -f dvtm ${OBJ} dvtm-${VERSION}.tar.gz init.cc precompiled_headers.h.gch

dist: clean
	@echo creating dist tarball
	@mkdir -p dvtm-${VERSION}
	@cp -R LICENSE Makefile README.md testsuite.sh config.def.h config.mk \
		${SRC} vt.h forkpty-aix.c forkpty-sunos.c tile.c bstack.c \
		tstack.c vstack.c grid.c fullscreen.c fibonacci.c \
		dvtm-status dvtm.info dvtm.1 dvtm-${VERSION}
	@tar -cf dvtm-${VERSION}.tar dvtm-${VERSION}
	@gzip dvtm-${VERSION}.tar
	@rm -rf dvtm-${VERSION}

install: dvtm
	@echo stripping executable
	@${STRIP} dvtm
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f dvtm ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/dvtm
	@cp -f dvtm-status ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/dvtm-status
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < dvtm.1 > ${DESTDIR}${MANPREFIX}/man1/dvtm.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/dvtm.1
	@echo installing terminfo description
	@TERMINFO=${TERMINFO} tic -s dvtm.info

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/dvtm
	@rm -f ${DESTDIR}${PREFIX}/bin/dvtm-status
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/dvtm.1

depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(CXX_SRC)

.PHONY: all options clean dist install uninstall debug

.SUFFIXES: .chai

$(DEPDIR)/%.d: ;

include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))