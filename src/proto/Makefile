include ../flags.makefile

# find ${PROTODIR} -iname '*.xml' -exec basename {} .xml \;
# without ge, xf86vidmode
PROTOS= \
	present \
	dpms \
	shm \
	damage \
	dri3 \
	render \
	xfixes \
	screensaver \
	bigreq \
	xprint \
	xinerama \
	xkb \
	xc_misc \
	record \
	xproto \
	xvmc \
	dri2 \
	xinput \
	sync \
	randr \
	xtest \
	res \
	shape \
	xevie \
	glx \
	xv \
	xselinux \
	xf86dri \
	composite

PYTHON=python2
CLIENT=cpp_client.py
XCBGEN=~/src/xcb/proto
PROTODIR=~/src/xcb/proto/src
XMLPROTOS=$(PROTOS:%=%.xml)
HPPPROTOS=$(XMLPROTOS:%.xml=%.hpp)

all: ${XMLPROTOS}

gch: ${HPPPROTOS}

${HPPPROTOS}:
	-${CXX} ${CXXFLAGS} ${LDFLAGS} -c $@

${XMLPROTOS}:
	-${PYTHON} ${CLIENT} -p ${XCBGEN} ${PROTODIR}/$@ > $(@:%.xml=%.hpp)

find:
	@ find ${PROTODIR} -iname '*.xml' -exec basename {} .xml \;

clean:
	rm -f ${PROTOS:%=%.hpp}
	rm -f $(PROTOS:%=%.hpp.gch)

.PHONY: clean find ${XMLPROTOS} ${HPPPROTOS}
